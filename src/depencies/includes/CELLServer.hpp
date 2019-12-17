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
	void HandleReadEvent(fd_set& fdRead);
	//当前 socket 触发了可写
	void HandleWriteEvent(fd_set& fdWrite);
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
	CELLLog::Info("CellServer %d::OnRun start\n", _id);
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

		//定时任务
		CheckTime();

		fd_set fdRead;
		fd_set fdWrite;

		if (_clientChange)
		{
			_clientChange = false;
			FD_ZERO(&fdRead);
			//nfds 当前 socket 最大值+1（兼容贝克利套接字）. 在 windows 里面可以设置为 0.
			_maxSocket = _clients[0]->getSocketfd();
			//把全局客户端数据加入可读监听部分
			for (int n = 0; n < _clients.size(); n++)
			{
				FD_SET(_clients[n]->getSocketfd(), &fdRead);
				if (_maxSocket < _clients[n]->getSocketfd())
				{
					_maxSocket = _clients[n]->getSocketfd();
				}
			}
			memcpy(&_fdReadBack, &fdRead, sizeof(fd_set));
		}
		else {
			memcpy(&fdRead, &_fdReadBack, sizeof(fd_set));
		}

		//检查需要可写数据到客户端
		bool bNeedWrite = false;
		FD_ZERO(&fdWrite);
		for (int n = 0; n < _clients.size(); n++)
		{
			if (_clients[n]->NeedWrite())
			{
				bNeedWrite = true;
				FD_SET(_clients[n]->getSocketfd(), &fdWrite);
			}
		}

		//memcpy(&fdWrite, &fdRead, sizeof(fd_set));
		/*
		NULL:一直阻塞
		timeval 只能精确到秒
		*/
		timeval t = { 0, 1 };
		int ret = 0;
		if (bNeedWrite)
		{
			ret = select((int)_maxSocket + 1, &fdRead, &fdWrite, nullptr, &t);
		}
		else {
			ret = select((int)_maxSocket + 1, &fdRead, nullptr, nullptr, &t);
		}
		
		if (SOCKET_ERROR == ret)
		{
			CELLLog::Info("CellServer %d::OnRun select error.\n", _id);
			pThread->Exit();
			break;
		}
		else if (0 == ret) {
			//CELLLog::Info("cellServer select timeout.\n");
			continue;
		}

		//处理可读
		HandleReadEvent(fdRead);
		//处理可写
		HandleWriteEvent(fdWrite);
	}
	CELLLog::Info("CellServer %d::OnRun end\n", _id);
}

inline void CellServer::HandleReadEvent(fd_set & fdRead)
{
	for (int n = 0; n < _clients.size(); n++)
	{
		SOCKET curfd = _clients[n]->getSocketfd();
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

inline void CellServer::HandleWriteEvent(fd_set & fdWrite)
{
	for (int n = 0; n < _clients.size(); n++)
	{
		SOCKET curfd = _clients[n]->getSocketfd();
		if (_clients[n]->NeedWrite() && FD_ISSET(curfd, &fdWrite))
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
	//读取消息
	int nLen = pClient->ReadData();
	if (nLen <= 0)
	{
		//CELLLog::Info("<sock=%d> CellServer::RecvData error.\n", (int)pClient->getSocketfd());
		return -1;
	}
	//统计接收消息次数
	if (_pNetEvent)
	{
		_pNetEvent->OnNetRecv(pClient);
	}
	//是否有一个消息
	while (pClient->HasMsg())
	{
		OnNetMsg(pClient, pClient->FrontMsg());
		pClient->PopFrontMsg();
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
	CELLLog::Info("CellServer %d::Close start\n", _id);
	_cellSendServer.Close();
	_thread.Close();
	CELLLog::Info("CellServer %d::Close end\n", _id);
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
