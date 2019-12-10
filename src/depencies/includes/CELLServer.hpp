#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include <vector>
#include <mutex>

#include "CELL.hpp"
#include "INetEvent.hpp"
#include "CELLClient.hpp"
#include "CELLTask.hpp"

class CellSendMsgTask : public CellTask
{
public:
	CellSendMsgTask(CELLClient* pClient, netmsg_DataHeader* pHeader);
	~CellSendMsgTask();
	void DoTask() override;
private:
	CELLClient* _pClient;
	netmsg_DataHeader* _pHeader;
};
CellSendMsgTask::CellSendMsgTask(CELLClient* pClient, netmsg_DataHeader* pHeader)
{
	this->_pClient = pClient;
	this->_pHeader = pHeader;
}
inline CellSendMsgTask::~CellSendMsgTask()
{
}
inline void CellSendMsgTask::DoTask()
{
	//发送消息
	_pClient->SendData(_pHeader);
	//删除消息
	delete _pHeader;
}
//////////////////////////////////////////////////////////////////////////
class CellServer
{
public:
	CellServer(SOCKET sock, INetEvent* netEvent);
	~CellServer();
	void AddClient(CELLClient* pClient);
	size_t GetClientCount();
	void Start();
	//添加发送任务
	void AddSendTask(CELLClient* pClient, netmsg_DataHeader* pHeader);
private:
	void OnRun();
	//是否运行
	bool IsRun();
	void Close();
	//接收数据
	int RecvData(CELLClient* pClient);
	//处理消息
	virtual void OnNetMsg(CELLClient* pClient, netmsg_DataHeader* pHeader);
private:
	SOCKET _sock = INVALID_SOCKET;
	INetEvent* _pNetEvent = nullptr;
	//当前正在处理的客户端队列
	std::vector<CELLClient*> _clients;
	//待处理队列
	std::vector<CELLClient*> _clientsBuf;
	//待处理队列锁
	std::mutex _mutex;

	//查询的客户端集合的备份
	fd_set _fdReadBack;
	//客户端数据有变化
	bool _clientChange = true;
	//当前最大的 socket
	SOCKET _maxSocket;
	//发送消息子服务
	CellTaskServer _cellSendServer;
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
inline void CellServer::AddClient(CELLClient * pClient)
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
	_cellSendServer.Start();
}

inline void CellServer::AddSendTask(CELLClient * pClient, netmsg_DataHeader * pHeader)
{
	CellSendMsgTask* pTask = new CellSendMsgTask(pClient, pHeader);
	_cellSendServer.AddTask(pTask);
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

int CellServer::RecvData(CELLClient* pClient)
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
#endif // _CELL_SERVER_HPP_
