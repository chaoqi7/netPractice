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
	int RecvData(SOCKET cSock);
	//处理消息
	virtual void OnNetMsg(SOCKET cSock, DataHeader* pHeader);
private:
	SOCKET _sock;
	std::vector<SOCKET> _clients;
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
		send(_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
	}
	//把新客户端 socket 添加到全局数据里面
	_clients.push_back(cSock);
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
			closesocket(_clients[n]);
		}
		//断开 socket 连接
		closesocket(_sock);
		WSACleanup();
#else
		for (int n = 0; n < _clients.size(); n++)
		{
			close(_clients[n]);
		}
		//断开 socket 连接
		close(_sock);
#endif
		_sock = INVALID_SOCKET;
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
		SendData(_clients[n], pHeader);
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
		FD_SET(_clients[n], &fdRead);
		if (maxSock < _clients[n])
		{
			maxSock = _clients[n];
		}
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
		if (FD_ISSET(_clients[n], &fdRead))
		{
			FD_CLR(_clients[n], &fdRead);
			if (-1 == RecvData(_clients[n]))
			{
#ifdef _WIN32
				closesocket(_clients[n]);
#else
				close(_clients[n]);
#endif
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

int EasyTcpServer::RecvData(SOCKET cSock)
{
	char msgBuf[1024] = {};
	int nLen = (int)recv(cSock, msgBuf, sizeof(DataHeader), 0);
	if (nLen <= 0)
	{
		printf("客户端<sock=%d>退出，任务结束.\n", (int)cSock);
		return -1;
	}
	DataHeader* pHeader = (DataHeader*)msgBuf;

	recv(cSock, msgBuf + sizeof(DataHeader), pHeader->dataLength - sizeof(DataHeader), 0);
	
	OnNetMsg(cSock, pHeader);

	return 0;
}

void EasyTcpServer::OnNetMsg(SOCKET cSock, DataHeader * pHeader)
{
	switch (pHeader->cmd)
	{
	case CMD_LOGIN:
	{		
		Login* pLogin = (Login*)pHeader;
		printf("收到命令:CMD_LOGIN, 数据长度:%d, userName:%s, password:%s\n",
			pLogin->dataLength, pLogin->userName, pLogin->passWord);
		//忽略登录消息的具体数据
		LoginResult loginResult;
		SendData(cSock, &loginResult);
	}
	break;
	case CMD_LOGINOUT:
	{
		Logout* pLogout = (Logout*)pHeader;
		printf("收到命令:CMD_LOGINOUT, 数据长度:%d, userName:%s\n",
			pLogout->dataLength, pLogout->userName);
		//忽略登出消息的具体数据
		LogoutResult loginoutResult;
		SendData(cSock, &loginoutResult);
	}
	break;
	default:
	{
		DataHeader dh;
		send(cSock, (const char*)&dh, sizeof(DataHeader), 0);
	}
	break;
	}
}

#endif // !_EASY_TCP_SERVER_HPP_
