#ifndef _EASY_TCP_SERVER_HPP_
#define _EASY_TCP_SERVER_HPP_

#ifdef _WIN32
//windows
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
//linux and osx
#include <unistd.h>
#include <arpa/inet.h>
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define SOCKET int
#endif // _WIN32

#include <stdio.h>
#include <vector>
#include "NetMsg.h"

class ClientSocket
{
public:
	ClientSocket(SOCKET cSock);
	~ClientSocket();
	SOCKET SocketFd();
	int GetLastMsgPos();
	void SetLastMsgPos(int newPos);
	char* MsgBuf();
private:
	SOCKET _cSock = INVALID_SOCKET;
	char _szMsgBuf[RECV_BUF_SIZE * 10] = {};
	int _lastMsgPos = 0;
};

ClientSocket::ClientSocket(SOCKET cSock)
{
	this->_cSock = cSock;
}

ClientSocket::~ClientSocket()
{
	_cSock = INVALID_SOCKET;
}

inline SOCKET ClientSocket::SocketFd()
{
	return _cSock;
}

inline int ClientSocket::GetLastMsgPos()
{
	return _lastMsgPos;
}

inline void ClientSocket::SetLastMsgPos(int newPos)
{
	_lastMsgPos = newPos;
}

inline char * ClientSocket::MsgBuf()
{
	return _szMsgBuf;
}
//////////////////////////////////////////////////////////////////////////
class EasyTcpServer
{
public:
	EasyTcpServer();
	virtual ~EasyTcpServer();
	//连接远程服务器
	int Bind(const char* ip, unsigned short port);
	//监听
	int Listen(int backlog);
	//关闭连接
	void Close();
	//循环执行任务（当前使用select)
	bool OnRun();
	//是否运行
	bool IsRun();
	//发送消息
	int SendData(SOCKET cSock, DataHeader* pHeader);
	void SendData2All(DataHeader* pHeader);
private:
	//初始化 socket
	void InitSocket();
	//接收客户端连接
	int Accept();
	//接收数据
	int RecvData(ClientSocket* pClient);
	//处理消息
	virtual void OnNetMsg(SOCKET cSock, DataHeader* pHeader);
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
};

EasyTcpServer::EasyTcpServer()
{
	_sock = INVALID_SOCKET;
}

EasyTcpServer::~EasyTcpServer()
{
	Close();
}

void EasyTcpServer::InitSocket()
{
	if (IsRun())
	{
		printf("InitSocket 关闭掉旧的 socket=%d\n", (int)_sock);
		Close();
	}

#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);
#endif
	//建立socket
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		printf("绑定端口失败.\n");
	}
	else {
		printf("绑定 socket=%d 成功.\n", (int)_sock);
	}
}

inline int EasyTcpServer::Bind(const char * ip, unsigned short port)
{
	if (!IsRun())
	{
		InitSocket();
	}
	sockaddr_in _sAddr = {};
	_sAddr.sin_family = AF_INET;
	_sAddr.sin_port = htons(port);
#ifdef _WIN32
	_sAddr.sin_addr.S_un.S_addr = ip ? inet_addr(ip) : INADDR_ANY;
#else
	_sAddr.sin_addr.s_addr = ip ? inet_addr(ip) : INADDR_ANY;
#endif
	int ret = bind(_sock, (sockaddr*)&_sAddr, sizeof(_sAddr));
	if (SOCKET_ERROR == ret)
	{
		printf("绑定<%s, %d>错误.\n", ip, port);
		return -1;
	}
	else {
		printf("绑定<%s, %d>成功.\n", ip, port);
	}
	return ret;
}

int EasyTcpServer::Listen(int backlog)
{
	if (IsRun())
	{
		int ret = listen(_sock, backlog);
		if (SOCKET_ERROR == ret)
		{
			printf("监听网络<socket=%d>端口错误.\n", (int)_sock);
			return -1;
		}
		else {
			printf("监听网络<socket=%d>端口成功.\n", (int)_sock);
		}
		return ret;
	}
	return SOCKET_ERROR;
}

int EasyTcpServer::Accept()
{
	sockaddr_in _cAddr = {};
#ifdef _WIN32
	int _cAddrLen = sizeof(sockaddr_in);
#else 
	socklen_t _cAddrLen = sizeof(sockaddr_in);
#endif // _WIN32
	SOCKET cSock = accept(_sock, (sockaddr*)&_cAddr, &_cAddrLen);
	if (INVALID_SOCKET == cSock)
	{
		printf("接受到无效客户端SOCKET\n");
		return -1;
	}
	//把新客户端登录消息广播给所有的客户端
	for (int n = 0; n < _clients.size(); n++)
	{
		NewUserJoin userJoin;
		userJoin.sock = (int)cSock;
		SendData(_clients[n]->SocketFd(), &userJoin);
	}
	//把新客户端 socket 添加到全局数据里面
	_clients.push_back(new ClientSocket(cSock));
	printf("接收到新客户端<sock:%d>: IP = %s\n", (int)cSock, inet_ntoa(_cAddr.sin_addr));

	return 0;
}

void EasyTcpServer::Close()
{
	if (IsRun())
	{
		//关闭所有的客户端连接
#ifdef _WIN32
		for (int n = 0; n < _clients.size(); n++)
		{
			closesocket(_clients[n]->SocketFd());
			delete _clients[n];
		}
		//断开 socket 连接
		closesocket(_sock);
		WSACleanup();
#else
		for (int n = 0; n < _clients.size(); n++)
		{
			close(_clients[n]->SocketFd());
			delete _clients[n];
		}
		//断开 socket 连接
		close(_sock);
#endif
		_sock = INVALID_SOCKET;
		_clients.clear();
	}
}

bool EasyTcpServer::IsRun()
{
	return _sock != INVALID_SOCKET;
}

int EasyTcpServer::SendData(SOCKET cSock, DataHeader * pHeader)
{
	int ret = SOCKET_ERROR;
	if (IsRun() && pHeader)
	{
		ret = send(cSock, (const char*)pHeader, pHeader->dataLength, 0);
		if (SOCKET_ERROR == ret)
		{
			printf("SendData error.\n");
		}
	}
	return ret;
}

void EasyTcpServer::SendData2All(DataHeader* pHeader)
{
	for (int n = 0; n < _clients.size(); n++)
	{
		SendData(_clients[n]->SocketFd(), pHeader);
	}
}

bool EasyTcpServer::OnRun()
{
	//nfds 当前 socket 最大值+1（兼容贝克利套接字）. 在 windows 里面可以设置为 0.
	SOCKET maxSock = _sock;
	fd_set fdRead;
	fd_set fdWrite;
	fd_set fdExcept;

	FD_ZERO(&fdRead);
	FD_ZERO(&fdWrite);
	FD_ZERO(&fdExcept);
	//把服务器 socket 加入监听
	FD_SET(_sock, &fdRead);
	FD_SET(_sock, &fdWrite);
	FD_SET(_sock, &fdExcept);

	//把全局客户端数据加入可读监听部分
	for (int n = 0; n < _clients.size(); n++)
	{
		FD_SET(_clients[n]->SocketFd(), &fdRead);
#ifndef _WIN32
		if (maxSock < _clients[n]->SocketFd())
		{
			maxSock = _clients[n]->SocketFd();
		}
#endif
	}
	/*
	NULL:一直阻塞
	timeval 只能精确到秒
	*/
	timeval t = { 1, 0 };
	int ret = select((int)maxSock + 1, &fdRead, &fdWrite, &fdExcept, &t);
	if (SOCKET_ERROR == ret)
	{
		printf("select error.\n");
		Close();
		return false;
	}
	else if (0 == ret) {
		//printf("select timeout.\n");
		//continue;
	}
	if (FD_ISSET(_sock, &fdRead))
	{
		FD_CLR(_sock, &fdRead);
		Accept();
	}

	//处理所有的客户端消息
	for (int n = 0; n < _clients.size(); n++)
	{
		if (FD_ISSET(_clients[n]->SocketFd(), &fdRead))
		{
			FD_CLR(_clients[n]->SocketFd(), &fdRead);
			if (-1 == RecvData(_clients[n]))
			{
#ifdef _WIN32
				closesocket(_clients[n]->SocketFd());
#else
				close(_clients[n]->SocketFd());
#endif
				delete _clients[n];
				//处理消息出现错误，在全局客户端数据里面删除它.
				auto iter = _clients.begin() + n;
				if (iter != _clients.end())
				{
					_clients.erase(iter);
				}
			}
		}
	}

	return true;
}

int EasyTcpServer::RecvData(ClientSocket* pClient)
{
	char szRecvBuf[RECV_BUF_SIZE] = {};
	int nLen = (int)recv(pClient->SocketFd(), szRecvBuf, RECV_BUF_SIZE, 0);
	if (nLen <= 0)
	{
		printf("客户端<sock=%d>退出，任务结束.\n", (int)pClient->SocketFd());
		return -1;
	}
	//把接收缓冲区的数据复制到消息缓冲区
	memcpy(pClient->MsgBuf() + pClient->GetLastMsgPos(), szRecvBuf, nLen);
	//当前未处理的消息长度 + nLen
	pClient->SetLastMsgPos(pClient->GetLastMsgPos() + nLen);
	//是否有一个消息头长度
	while (pClient->GetLastMsgPos() >= sizeof(DataHeader))
	{
		DataHeader* pHeader = (DataHeader*)pClient->MsgBuf();
		//是否有一条真正的消息长度
		if (pClient->GetLastMsgPos() >= pHeader->dataLength)
		{
			//剩余未处理的消息长度
			int nLeftMsgLen = pClient->GetLastMsgPos() - pHeader->dataLength;
			//处理消息
			OnNetMsg(pClient->SocketFd(), pHeader);
			if (nLeftMsgLen > 0)
			{
				//未处理的消息前移
				memcpy(pClient->MsgBuf(), pClient->MsgBuf() + pHeader->dataLength, nLeftMsgLen);
				//更新未处理消息长度
				pClient->SetLastMsgPos(nLeftMsgLen);
			}
			else {
				pClient->SetLastMsgPos(0);
			}			
		}
		else {
			break;
		}
	}

	return 0;
}

void EasyTcpServer::OnNetMsg(SOCKET cSock, DataHeader * pHeader)
{
	switch (pHeader->cmd)
	{
	case CMD_LOGIN:
	{		
		Login* pLogin = (Login*)pHeader;
		//printf("收到命令:CMD_LOGIN, 数据长度:%d, userName:%s, password:%s\n",
		//	pLogin->dataLength, pLogin->userName, pLogin->passWord);
		//忽略登录消息的具体数据
		LoginResult loginResult;
		SendData(cSock, &loginResult);
	}
	break;
	case CMD_LOGINOUT:
	{
		Logout* pLogout = (Logout*)pHeader;
		//printf("收到命令:CMD_LOGINOUT, 数据长度:%d, userName:%s\n",
		//	pLogout->dataLength, pLogout->userName);
		//忽略登出消息的具体数据
		LogoutResult loginoutResult;
		SendData(cSock, &loginoutResult);
	}
	break;
	default:
	{
		printf("收到未定义消息.\n");
		DataHeader dh;
		SendData(cSock, &dh);
	}
	break;
	}
}

#endif // !_EASY_TCP_SERVER_HPP_
