#ifndef _EASY_TCP_SERVER_HPP_
#define _EASY_TCP_SERVER_HPP_

#include <atomic>

#include "CELL.hpp"
#include "CELLTimeStamp.hpp"
#include "CELLClient.hpp"
#include "CELLServer.hpp"
#include "CELLThread.hpp"
#include "CELLNetWork.hpp"

class EasyTcpServer : public INetEvent
{
public:
	EasyTcpServer(int nSendSize, int nRecvSize);
	virtual ~EasyTcpServer();
	//连接远程服务器
	int Bind(const char* ip, unsigned short port);
	//监听
	int Listen(int backlog);
	//关闭连接
	void Close();
	//创建工作子线程
	void Start(int cellServerCount = 1);
public:
	//继承的接口
	void OnNetJoin(CELLClient* pClient) override
	{
		_clientCount++;
		//CELLLog::Info("EasyTcpServer OnNetJoin.......\n");
	}

	void OnNetLeave(CELLClient* pClient) override
	{
		_clientCount--;
		//CELLLog::Info("EasyTcpServer OnNetLeave.......\n");
	}

	void OnNetMsg(CellServer* pCellServer, CELLClient* pClient, netmsg_DataHeader* pHeader) override
	{
		_msgCount++;
		//CELLLog::Info("EasyTcpServer OnNetMsg.......\n");
	}
	void OnNetRecv(CELLClient* pClient) override
	{
		_recvCount++;
	}
private:
	//初始化 socket
	void InitSocket();
	//接收客户端连接
	int Accept();
	//添加新客户端到子线程
	void AddClient2CellServer(CELLClient* pClient);
	//工作函数
	void OnRun(CELLThread* pThread);
	//统计包数据
	void time4msg();
private:
	SOCKET _sock;
	//子线程队列
	std::vector<CellServer*> _cellServers;
	CELLTimeStamp _tTime;
	CELLThread _thread;
	//为客户端分配的发送缓冲区大小
	int _nSendBufSize = 0;
	//为客户端分配的接收缓冲区大小
	int _nRecvBufSize = 0;
protected:
	std::atomic<int> _clientCount{ 0 };
	std::atomic<int> _msgCount{ 0 };
	std::atomic<int> _recvCount{ 0 };
};

EasyTcpServer::EasyTcpServer(int nSendSize, int nRecvSize)
{
	_sock = INVALID_SOCKET;
	_nSendBufSize = nSendSize;
	_nRecvBufSize = nRecvSize;
}

EasyTcpServer::~EasyTcpServer()
{
	Close();
}

void EasyTcpServer::InitSocket()
{
	if (_sock != INVALID_SOCKET)
	{
		CELLLog::Info("InitSocket 关闭掉旧的 socket=%d\n", (int)_sock);
		Close();
	}

	CELLNetWork::Init();
	//建立socket
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		CELLLog::Info("绑定端口失败.\n");
	}
	else {
		CELLLog::Info("绑定 socket=%d 成功.\n", (int)_sock);
	}
}

inline int EasyTcpServer::Bind(const char * ip, unsigned short port)
{
	if (_sock == INVALID_SOCKET)
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
		CELLLog::Info("绑定<%s, %d>错误.\n", ip, port);
		return -1;
	}
	else {
		CELLLog::Info("绑定<%s, %d>成功.\n", ip, port);
	}
	return ret;
}

int EasyTcpServer::Listen(int backlog)
{
	int ret = SOCKET_ERROR;
	if (_sock != INVALID_SOCKET)
	{
		ret = listen(_sock, backlog);
		if (SOCKET_ERROR == ret)
		{
			CELLLog::Info("监听网络<socket=%d>端口错误.\n", (int)_sock);
		}
		else {
			CELLLog::Info("监听网络<socket=%d>端口成功.\n", (int)_sock);
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
		CELLLog::Info("接受到无效客户端SOCKET\n");
		return -1;
	}
	//把新客户端 socket 添加到全局数据里面
	AddClient2CellServer(new CELLClient(cSock, _nSendBufSize, _nRecvBufSize));
	return 0;
}

void EasyTcpServer::Close()
{
	CELLLog::Info("EasyTcpServer::Close start\n");
	_thread.Close();
	if (_sock != INVALID_SOCKET)
	{
		for (auto ser : _cellServers)
		{
			delete ser;
		}
		_cellServers.clear();
#ifdef _WIN32
		//断开 socket 连接
		closesocket(_sock);
#else
		//断开 socket 连接
		close(_sock);
#endif
		_sock = INVALID_SOCKET;
	}
	CELLLog::Info("EasyTcpServer::Close end\n");
}

void EasyTcpServer::OnRun(CELLThread* pThread)
{
	while (pThread->IsRun())
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
			CELLLog::Info("EasyTcpServer::OnRun select error.\n");
			pThread->Exit();
			break;
		}
		else if (0 == ret) {
			//CELLLog::Info("select timeout.\n");
			//continue;
		}

		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			Accept();
		}
	}
}

inline void EasyTcpServer::Start(int cellServerCount)
{
	for (int n = 0; n < cellServerCount; n++)
	{
		auto ser = new CellServer(n + 1, this);
		_cellServers.push_back(ser);
		ser->Start();
	}

	_thread.Start(nullptr,[this](CELLThread* pThread){
		OnRun(pThread);
	}, nullptr);
}

inline void EasyTcpServer::AddClient2CellServer(CELLClient * pClient)
{
	//找出最小客户端数据，添加进去
	CellServer* pMinServer = _cellServers[0];
	for (size_t n = 1; n < _cellServers.size(); n++)
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
		CELLLog::Info("time<%lf>, thread<%d>, clients<%d>, recv<%d>, msg<%d>\n",
			t, (int)_cellServers.size(), (int)_clientCount, (int)_recvCount, (int)_msgCount);
		_tTime.update();
		_msgCount = 0;
		_recvCount = 0;
	}
}
#endif // !_EASY_TCP_SERVER_HPP_
