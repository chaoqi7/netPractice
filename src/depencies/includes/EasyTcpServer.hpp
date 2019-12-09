#ifndef _EASY_TCP_SERVER_HPP_
#define _EASY_TCP_SERVER_HPP_

#ifdef _WIN32
//windows
#define FD_SETSIZE      4024
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
#include <thread>
#include <mutex>
#include <atomic>
#include "NetMsg.h"
#include "CELLTimeStamp.hpp"

class ClientSocket
{
public:
	ClientSocket(SOCKET cSock);
	~ClientSocket();
	//获取当前客户端的 socket
	SOCKET GetSocketfd();
	//获取当前消息的最后索引
	int GetLastRecvPos();
	//设置当前消息的最后索引
	void SetLastRecvPos(int newPos);
	//获取当前消息缓冲区
	char* RecvBuf();
	//关闭当前客户端的 socket
	void Close();
	//发送消息
	int SendData(DataHeader* pHeader);
private:
	SOCKET _cSock = INVALID_SOCKET;
	//接收消息缓冲区
	char _szRecvBuf[RECV_BUF_SIZE] = {};
	//接收消息缓冲区位置
	int _lastRecvPos = 0;
	//发送缓冲区
	char _szSendBuf[SEND_BUF_SIZE] = {};
	//发送缓冲区位置
	int _lastSendPos = 0;
};

ClientSocket::ClientSocket(SOCKET cSock)
{
	this->_cSock = cSock;
}

ClientSocket::~ClientSocket()
{
	_cSock = INVALID_SOCKET;
}

inline SOCKET ClientSocket::GetSocketfd()
{
	return _cSock;
}

inline int ClientSocket::GetLastRecvPos()
{
	return _lastRecvPos;
}

inline void ClientSocket::SetLastRecvPos(int newPos)
{
	_lastRecvPos = newPos;
}

inline char * ClientSocket::RecvBuf()
{
	return _szRecvBuf;
}
inline void ClientSocket::Close()
{
	if (_cSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_cSock);
#else
		close(_cSock);
#endif
		_cSock = INVALID_SOCKET;
	}	
}
inline int ClientSocket::SendData(DataHeader * pHeader)
{
	int ret = SOCKET_ERROR;
	int nSendLen = pHeader->dataLength;
	const char* pSendData = (const char*)pHeader;

	while (true)
	{		
		if ((_lastSendPos + nSendLen) >= SEND_BUF_SIZE)
		{
			int nCopyLen = SEND_BUF_SIZE - _lastSendPos;
			if (nCopyLen > 0)
			{
				//复制数据
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//修正位置
				pSendData += nCopyLen;
				nSendLen -= nCopyLen;
			}

			//发送数据
			ret = send(_cSock, _szSendBuf, SEND_BUF_SIZE, 0);
			//清空缓冲区
			_lastSendPos = 0;
			if (SOCKET_ERROR == ret)
			{
				printf("sock=%d sendData fail.\n", (int)_cSock);
				return ret;
			}			
		}
		else {
			//消息没有达到缓冲区最大，就只是复制数据
			memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
			_lastSendPos += nSendLen;
			ret = nSendLen;
			break;
		}
	}

	return ret;
}
//////////////////////////////////////////////////////////////////////////
class INetEvent
{
public:
	//网络断开
	virtual void OnNetLeave(ClientSocket* pClient) = 0;
	//网络建立
	virtual void OnNetJoin(ClientSocket* pClient) = 0;
	//网络消息
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* pHeader) = 0;
};
//////////////////////////////////////////////////////////////////////////
class CellServer
{
public:
	CellServer(SOCKET sock, INetEvent* netEvent);
	~CellServer();
	void AddClient(ClientSocket* pClient);
	size_t GetClientCount();
	void Start();
private:
	void OnRun();
	//是否运行
	bool IsRun();
	void Close();
	//接收数据
	int RecvData(ClientSocket* pClient);
	//处理消息
	virtual void OnNetMsg(ClientSocket* pClient, DataHeader* pHeader);
private:
	SOCKET _sock = INVALID_SOCKET;
	INetEvent* _pNetEvent = nullptr;
	//当前正在处理的客户端队列
	std::vector<ClientSocket*> _clients;
	//待处理队列
	std::vector<ClientSocket*> _clientsBuf;
	//待处理队列锁
	std::mutex _mutex;

	//查询的客户端集合的备份
	fd_set _fdReadBack;
	//客户端数据有变化
	bool _clientChange = true;
	//当前最大的 socket
	SOCKET _maxSocket;
};

inline CellServer::CellServer(SOCKET sock, INetEvent * netEvent)
{
	_sock = sock;
	_pNetEvent = netEvent;
}

CellServer::~CellServer()
{
	Close();
}
inline void CellServer::AddClient(ClientSocket * pClient)
{
	////把新客户端登录消息广播给所有的客户端
	//NewUserJoin userJoin;
	//userJoin.sock = (int)pClient->GetSocketfd();
	//SendData2All(&userJoin);
	if (_pNetEvent)
	{
		_pNetEvent->OnNetJoin(pClient);
	}
	//加入到待处理队列
	std::lock_guard<std::mutex> lock(_mutex);
	_clientsBuf.push_back(pClient);
}
inline size_t CellServer::GetClientCount()
{
	return _clientsBuf.size() + _clients.size();
}
inline void CellServer::Start()
{
	std::thread t(std::mem_fn(&CellServer::OnRun), this);
	t.detach();
}

inline void CellServer::OnRun()
{
	while (IsRun())
	{
		if (!_clientsBuf.empty())
		{
			std::lock_guard<std::mutex> lock(_mutex);
			for (auto pNewClient : _clientsBuf)
			{
				_clients.push_back(pNewClient);
			}
			_clientsBuf.clear();
			_clientChange = true;
		}		

		if (_clients.empty())
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
			continue;
		}

		fd_set fdRead;
		FD_ZERO(&fdRead);
		if (_clientChange)
		{
			_clientChange = false;
			//nfds 当前 socket 最大值+1（兼容贝克利套接字）. 在 windows 里面可以设置为 0.
			_maxSocket = _clients[0]->GetSocketfd();
			//把全局客户端数据加入可读监听部分
			for (int n = 0; n < _clients.size(); n++)
			{
				FD_SET(_clients[n]->GetSocketfd(), &fdRead);
				if (_maxSocket < _clients[n]->GetSocketfd())
				{
					_maxSocket = _clients[n]->GetSocketfd();
				}
			}
			memcpy(&_fdReadBack, &fdRead, sizeof(fd_set));
		}
		else {
			memcpy(&fdRead, &_fdReadBack, sizeof(fd_set));
		}

		/*
		NULL:一直阻塞
		timeval 只能精确到秒
		*/
		timeval t = { 0, 1 };
		int ret = select((int)_maxSocket + 1, &fdRead, nullptr, nullptr, &t);
		if (SOCKET_ERROR == ret)
		{
			printf("cellServer select error.\n");
			Close();
			break;
		}
		else if (0 == ret) {
			//printf("cellServer select timeout.\n");
			//continue;
		}

		//处理所有的客户端消息
		for (int n = 0; n < _clients.size(); n++)
		{
			if (FD_ISSET(_clients[n]->GetSocketfd(), &fdRead))
			{
				FD_CLR(_clients[n]->GetSocketfd(), &fdRead);
				if (-1 == RecvData(_clients[n]))
				{
					_clients[n]->Close();
					//处理消息出现错误，在全局客户端数据里面删除它.
					auto iter = _clients.begin() + n;
					if (iter != _clients.end())
					{
						_clientChange = true;
						if (_pNetEvent)
						{
							_pNetEvent->OnNetLeave(_clients[n]);
						}
						delete _clients[n];
						_clients.erase(iter);
					}
				}
			}
		}
	}
}
inline bool CellServer::IsRun()
{
	return _sock != INVALID_SOCKET;
}
inline void CellServer::Close()
{
	if (IsRun())
	{
		//关闭所有的客户端连接
		for (int n = 0; n < _clients.size(); n++)
		{
			_clients[n]->Close();
			delete _clients[n];
		}
		_clients.clear();

		for (int n = 0; n < _clientsBuf.size(); n++)
		{
			_clientsBuf[n]->Close();
			delete _clientsBuf[n];
		}
		_clientsBuf.clear();
	}
}

int CellServer::RecvData(ClientSocket* pClient)
{
	char* szRecvBuf = pClient->RecvBuf() + pClient->GetLastRecvPos();
	int nLen = (int)recv(pClient->GetSocketfd(), szRecvBuf, RECV_BUF_SIZE - pClient->GetLastRecvPos(), 0);
	if (nLen <= 0)
	{
		//printf("客户端<sock=%d>退出，任务结束.\n", (int)pClient->GetSocketfd());
		return -1;
	}
	//当前未处理的消息长度 + nLen
	pClient->SetLastRecvPos(pClient->GetLastRecvPos() + nLen);
	//是否有一个消息头长度
	while (pClient->GetLastRecvPos() >= sizeof(DataHeader))
	{
		DataHeader* pHeader = (DataHeader*)pClient->RecvBuf();
		//是否有一条真正的消息长度
		if (pClient->GetLastRecvPos() >= pHeader->dataLength)
		{
			//剩余未处理的消息长度
			int nLeftMsgLen = pClient->GetLastRecvPos() - pHeader->dataLength;
			//处理消息
			OnNetMsg(pClient, pHeader);
			if (nLeftMsgLen > 0)
			{
				//未处理的消息前移
				memcpy(pClient->RecvBuf(), pClient->RecvBuf() + pHeader->dataLength, nLeftMsgLen);
				//更新未处理消息长度
				pClient->SetLastRecvPos(nLeftMsgLen);
			}
			else {
				pClient->SetLastRecvPos(0);
			}
		}
		else {
			break;
		}
	}

	return 0;
}

void CellServer::OnNetMsg(ClientSocket* pClient, DataHeader * pHeader)
{
	if (_pNetEvent)
	{
		_pNetEvent->OnNetMsg(pClient, pHeader);
	}
}
//////////////////////////////////////////////////////////////////////////
class EasyTcpServer : public INetEvent
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
	//创建工作子线程
	void Start(int cellServerCount = 1);
public:
	//继承的接口
	void OnNetJoin(ClientSocket* pClient) override
	{
		_clientCount++;
		//printf("EasyTcpServer OnNetJoin.......\n");
	}

	void OnNetLeave(ClientSocket* pClient) override
	{
		_clientCount--;
		//printf("EasyTcpServer OnNetLeave.......\n");
	}

	void OnNetMsg(ClientSocket* pClient, DataHeader* pHeader) override
	{
		_msgCount++;
		//printf("EasyTcpServer OnNetMsg.......\n");
	}
private:
	//初始化 socket
	void InitSocket();
	//接收客户端连接
	int Accept();
	//添加新客户端到子线程
	void AddClient2CellServer(ClientSocket* pClient);
	void time4msg();
private:
	SOCKET _sock;
	//子线程队列
	std::vector<CellServer*> _cellServers;
	CELLTimeStamp _tTime;
protected:
	std::atomic<int> _clientCount = 0;
	std::atomic<int> _msgCount = 0;
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
	int ret = SOCKET_ERROR;
	if (IsRun())
	{
		ret = listen(_sock, backlog);
		if (SOCKET_ERROR == ret)
		{
			printf("监听网络<socket=%d>端口错误.\n", (int)_sock);
		}
		else {
			printf("监听网络<socket=%d>端口成功.\n", (int)_sock);
		}
	}

	return ret;
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
	//把新客户端 socket 添加到全局数据里面
	AddClient2CellServer(new ClientSocket(cSock));
	return 0;
}

void EasyTcpServer::Close()
{
	if (IsRun())
	{
#ifdef _WIN32
		//断开 socket 连接
		closesocket(_sock);
		WSACleanup();
#else
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

bool EasyTcpServer::OnRun()
{
	time4msg();
	fd_set fdRead;
	FD_ZERO(&fdRead);
	//把服务器 socket 加入监听
	FD_SET(_sock, &fdRead);
	/*
	NULL:一直阻塞
	timeval 只能精确到秒
	*/
	timeval t = { 0, 1 };
	int ret = select((int)_sock + 1, &fdRead, nullptr, nullptr, &t);
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

	return true;
}

inline void EasyTcpServer::Start(int cellServerCount)
{
	for (int n = 0; n < cellServerCount; n++)
	{
		auto ser = new CellServer(_sock, this);
		_cellServers.push_back(ser);
		ser->Start();
	}
}

inline void EasyTcpServer::AddClient2CellServer(ClientSocket * pClient)
{
	//找出最小客户端数据，添加进去
	CellServer* pMinServer = _cellServers[0];
	for (int n = 1; n < _cellServers.size(); n++)
	{
		if (pMinServer->GetClientCount() > _cellServers[n]->GetClientCount())
		{
			pMinServer = _cellServers[n];
		}
	}
	pMinServer->AddClient(pClient);
}

inline void EasyTcpServer::time4msg()
{
	auto t = _tTime.getElapseTimeInSeconds();
	if (t > 1.0)
	{
		printf("time<%lf>, thread<%d>, clients<%d>, msg<%d>\n",
			t, _cellServers.size(), _clientCount, _msgCount);
		_tTime.update();
		_msgCount = 0;
	}
}

#endif // !_EASY_TCP_SERVER_HPP_
