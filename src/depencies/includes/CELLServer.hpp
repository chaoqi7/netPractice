#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include <vector>
#include <mutex>

#include "CELL.hpp"
#include "INetEvent.hpp"
#include "CELLClient.hpp"
#include "CELLTask.hpp"
#include "CELLThread.hpp"

class CellServer
{
public:
	CellServer(int id, INetEvent* netEvent);
	~CellServer();
	void AddClient(CELLClient* pClient);
	size_t GetClientCount();
	void Start();
	//添加发送任务
	void AddSendTask(CELLClient* pClient, netmsg_DataHeader* pHeader);
private:
	void OnRun(CELLThread* pThread);
	void Close();
	void CleanClients();
	//当前 socket 触发了可读
	void ReadData(fd_set& fdRead);
	//当前 socket 触发了可写
	void WriteData(fd_set& fdWrite);
	//有客户端退出
	void OnClientLeave(CELLClient* pClient);
	//接收数据
	int RecvData(CELLClient* pClient);
	//处理消息
	virtual void OnNetMsg(CELLClient* pClient, netmsg_DataHeader* pHeader);
	//定时任务
	void CheckTime();
private:
	INetEvent* _pNetEvent = nullptr;
	//当前正在处理的客户端队列
	std::vector<CELLClient*> _clients;
	//待处理队列
	std::vector<CELLClient*> _clientsBuf;
	//待处理队列锁
	std::mutex _mutex;

	//查询的客户端集合的备份
	fd_set _fdReadBack;

	//当前最大的 socket
	SOCKET _maxSocket;
	//发送消息子服务
	CellTaskServer _cellSendServer;
	//当前服务启动的时间
	long long _oldTime = CELLTime::getNowInMilliseconds();
	//服务器ID
	int _id = -1;
	//客户端数据有变化
	bool _clientChange = true;
	//子线程
	CELLThread _thread;
};

inline CellServer::CellServer(int id, INetEvent * netEvent)
{
	_id = id;
	_pNetEvent = netEvent;
}

CellServer::~CellServer()
{
	Close();
}
inline void CellServer::AddClient(CELLClient * pClient)
{
	////把新客户端登录消息广播给所有的客户端
	//NewUserJoin userJoin;
	//userJoin.sock = (int)pClient->GetSocketfd();
	//SendData2All(&userJoin);
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
	_cellSendServer.Start(_id);
	_thread.Start(nullptr, [this](CELLThread* pThread) {
		OnRun(pThread);
	}, [this](CELLThread* pThread) {
		CleanClients();
	});
}

inline void CellServer::AddSendTask(CELLClient * pClient, netmsg_DataHeader * pHeader)
{
	_cellSendServer.AddTask([pClient, pHeader]() {
		if (pClient && pHeader)
		{
			pClient->SendData(pHeader);
			delete pHeader;
		}
	});
}

inline void CellServer::OnRun(CELLThread* pThread)
{
	printf("CellServer %d::OnRun start\n", _id);
	while (pThread->IsRun())
	{
		if (!_clientsBuf.empty())
		{
			std::lock_guard<std::mutex> lock(_mutex);
			for (auto pNewClient : _clientsBuf)
			{
				pNewClient->SetServerID(_id);
				_clients.push_back(pNewClient);
				if (_pNetEvent)
				{
					_pNetEvent->OnNetJoin(pNewClient);
				}
			}
			_clientsBuf.clear();
			_clientChange = true;
		}

		if (_clients.empty())
		{
			std::chrono::milliseconds t(1);
			std::this_thread::sleep_for(t);
			//更新旧时间戳
			_oldTime = CELLTime::getNowInMilliseconds();
			continue;
		}

		fd_set fdRead;
		fd_set fdWrite;

		if (_clientChange)
		{
			_clientChange = false;
			FD_ZERO(&fdRead);
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

		memcpy(&fdWrite, &fdRead, sizeof(fd_set));
		/*
		NULL:一直阻塞
		timeval 只能精确到秒
		*/
		timeval t = { 0, 1 };
		int ret = select((int)_maxSocket + 1, &fdRead, &fdWrite, nullptr, &t);
		if (SOCKET_ERROR == ret)
		{
			printf("CellServer %d::OnRun select error.\n", _id);
			pThread->Exit();
			break;
		}
		else if (0 == ret) {
			//printf("cellServer select timeout.\n");
			//continue;
		}

		//处理可读
		ReadData(fdRead);
		//处理可写
		WriteData(fdWrite);		
		//定时任务
		CheckTime();
	}
	printf("CellServer %d::OnRun end\n", _id);
}

inline void CellServer::ReadData(fd_set & fdRead)
{
	for (int n = 0; n < _clients.size(); n++)
	{
		SOCKET curfd = _clients[n]->GetSocketfd();
		if (FD_ISSET(curfd, &fdRead))
		{
			FD_CLR(curfd, &fdRead);
			if (-1 == RecvData(_clients[n]))
			{
				//处理消息出现错误，在全局客户端数据里面删除它.
				auto iter = _clients.begin() + n;
				if (iter != _clients.end())
				{
					OnClientLeave(_clients[n]);
					_clients.erase(iter);
				}
			}
		}
	}
}

inline void CellServer::WriteData(fd_set & fdWrite)
{
	for (int n = 0; n < _clients.size(); n++)
	{
		SOCKET curfd = _clients[n]->GetSocketfd();
		if (FD_ISSET(curfd, &fdWrite))
		{
			FD_CLR(curfd, &fdWrite);
			if (SOCKET_ERROR == _clients[n]->SendDataReal())
			{
				auto iter = _clients.begin() + n;
				if (iter != _clients.end())
				{
					OnClientLeave(_clients[n]);
					_clients.erase(iter);
				}
			}
		}
	}
}
void CellServer::CheckTime()
{
	auto tNewTime = CELLTime::getNowInMilliseconds();
	auto dt = tNewTime - _oldTime;
	_oldTime = tNewTime;

	for (int n = 0; n < _clients.size(); n++)
	{
		//定时存活检测
		if (_clients[n]->CheckHeart(dt))
		{
			OnClientLeave(_clients[n]);
			_clients.erase(_clients.begin() + n);
			continue;
		}

		//定时发送检测
		_clients[n]->CheckSend(dt);
	}
}
inline void CellServer::OnClientLeave(CELLClient * pClient)
{
	_clientChange = true;
	if (_pNetEvent)
	{
		_pNetEvent->OnNetLeave(pClient);
	}
	delete pClient;
}

int CellServer::RecvData(CELLClient* pClient)
{
	char* szRecvBuf = pClient->RecvBuf() + pClient->GetLastRecvPos();
	int nLen = (int)recv(pClient->GetSocketfd(), szRecvBuf, RECV_BUF_SIZE - pClient->GetLastRecvPos(), 0);
	if (nLen <= 0)
	{
		//printf("客户端<sock=%d>退出，任务结束.\n", (int)pClient->GetSocketfd());
		return -1;
	}
	if (_pNetEvent)
	{
		_pNetEvent->OnNetRecv(pClient);
	}
	//当前未处理的消息长度 + nLen
	pClient->SetLastRecvPos(pClient->GetLastRecvPos() + nLen);
	//是否有一个消息头长度
	while (pClient->GetLastRecvPos() >= sizeof(netmsg_DataHeader))
	{
		netmsg_DataHeader* pHeader = (netmsg_DataHeader*)pClient->RecvBuf();
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

void CellServer::OnNetMsg(CELLClient* pClient, netmsg_DataHeader * pHeader)
{
	if (_pNetEvent)
	{
		_pNetEvent->OnNetMsg(this, pClient, pHeader);
	}
}

inline void CellServer::Close()
{
	printf("CellServer %d::Close start\n", _id);
	_cellSendServer.Close();
	_thread.Close();
	printf("CellServer %d::Close end\n", _id);
}

inline void CellServer::CleanClients()
{
	//关闭所有的客户端连接
	for (int n = 0; n < _clients.size(); n++)
	{
		delete _clients[n];
	}
	_clients.clear();

	for (int n = 0; n < _clientsBuf.size(); n++)
	{
		delete _clientsBuf[n];
	}
	_clientsBuf.clear();
}
#endif // _CELL_SERVER_HPP_
