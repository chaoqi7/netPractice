#ifndef _CELL_SERVER_HPP_
#define _CELL_SERVER_HPP_

#include <vector>
#include <mutex>

#include "CELL.hpp"
#include "INetEvent.hpp"
#include "CELLClient.hpp"
#include "CELLTask.hpp"
#include "CELLThread.hpp"
#include "CELLTimeStamp.hpp"

class CellServer
{
public:
	virtual ~CellServer();
	void setObj(int id, INetEvent *netEvent);
	void AddClient(CELLClient *pClient);
	size_t GetClientCount();
	void Start();
	//添加发送任务
	void AddSendTask(CELLClient *pClient, netmsg_DataHeader *pHeader);

protected:
	//通过select 检测可写，可读。
	virtual bool DoNetEvents() = 0;
	void Close();

private:
	void OnRun(CELLThread *pThread);
	void CleanClients();
	//处理消息
	void DoMsg();
	//处理消息
	virtual void OnNetMsg(CELLClient *pClient, netmsg_DataHeader *pHeader);
	//定时任务
	void CheckTime();

protected:
	//接收数据
	int RecvData(CELLClient *pClient);

	void OnNetRecv(CELLClient* pClient);
	//有客户端退出
	void OnClientLeave(CELLClient *pClient);
	//
	virtual void OnClientJoin(CELLClient *pClient);

protected:
	//客户端数据有变化
	bool _clientChange = true;
	//当前正在处理的客户端队列
	std::vector<CELLClient *> _clients;
	//待处理队列
	std::vector<CELLClient *> _clientsBuf;

private:
	INetEvent *_pNetEvent = nullptr;
	//待处理队列锁
	std::mutex _mutex;
	//发送消息子服务
	CellTaskServer _cellSendServer;
	//当前服务启动的时间
	long long _oldTime = CELLTime::getNowInMilliseconds();
	//服务器ID
	int _id = -1;

	//子线程
	CELLThread _thread;
};

CellServer::~CellServer()
{
	Close();
}

void CellServer::setObj(int id, INetEvent *netEvent)
{
	_id = id;
	_pNetEvent = netEvent;
}

inline void CellServer::AddClient(CELLClient *pClient)
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
	_thread.Start(nullptr, [this](CELLThread *pThread) { OnRun(pThread); }, [this](CELLThread *pThread) { CleanClients(); });
}

inline void CellServer::AddSendTask(CELLClient *pClient, netmsg_DataHeader *pHeader)
{
	_cellSendServer.AddTask([pClient, pHeader]() {
		if (pClient && pHeader)
		{
			pClient->SendData(pHeader);
			delete pHeader;
		}
	});
}

inline void CellServer::DoMsg()
{
	CELLClient *pClient = nullptr;
	for (size_t n = 0; n < _clients.size(); n++)
	{
		pClient = _clients[n];
		if (pClient)
		{
			//是否有一个消息
			/*
			可以每次处理一条消息，或者一次处理完当前客户端的所有消息
			*/
			while (pClient->HasMsg())
			{
				OnNetMsg(pClient, pClient->FrontMsg());
				pClient->PopFrontMsg();
			}
		}
	}
}

inline void CellServer::OnRun(CELLThread *pThread)
{
	CELLLog_Debug("CellServer %d::OnRun start", _id);
	while (pThread->IsRun())
	{
		if (!_clientsBuf.empty())
		{
			std::lock_guard<std::mutex> lock(_mutex);
			for (auto pNewClient : _clientsBuf)
			{
				pNewClient->SetServerID(_id);
				_clients.push_back(pNewClient);
				OnClientJoin(pNewClient);
			}
			_clientsBuf.clear();
			_clientChange = true;
		}

		if (_clients.empty())
		{
			CELLThread::Sleep(1);
			//更新旧时间戳
			_oldTime = CELLTime::getNowInMilliseconds();
			continue;
		}

		//定时任务
		CheckTime();
		//检测可读，可写。
		if (!DoNetEvents())
		{
			CELLLog_Error("CellServer %d::OnRun select error.", _id);
			pThread->Exit();
			break;
		}
		//处理具体的消息
		DoMsg();
	}
	CELLLog_Debug("CellServer %d::OnRun end", _id);
}

void CellServer::CheckTime()
{
	auto tNewTime = CELLTime::getNowInMilliseconds();
	auto dt = tNewTime - _oldTime;
	_oldTime = tNewTime;

	CELLClient* pClient = nullptr;
	for (size_t n = 0; n < _clients.size(); n++)
	{
		pClient = _clients[n];
		if (pClient)
		{
			//定时存活检测
			if (pClient->CheckHeart(dt))
			{
#ifdef _USE_IOCP_
				//如果投递了IO操作，则只是关闭 socket，上层处理删除客户端
				if (pClient->isInIoAction())
				{
					pClient->Close();
				}
				else
				{
					OnClientLeave(pClient);
				}
#else
				OnClientLeave(pClient);
#endif

				_clients.erase(_clients.begin() + n);
				continue;
				}

			//定时发送检测
			//pClient->CheckSend(dt);
		}
	}
}
inline void CellServer::OnClientLeave(CELLClient *pClient)
{
	CELLLog_Debug("CellServer::OnClientLeave fd=%d", pClient->socketfd());
	_clientChange = true;
	if (_pNetEvent)
	{
		_pNetEvent->OnNetLeave(pClient);
	}
	delete pClient;
}

inline void CellServer::OnClientJoin(CELLClient *pClient)
{
	if (_pNetEvent)
	{
		_pNetEvent->OnNetJoin(pClient);
	}
}

int CellServer::RecvData(CELLClient *pClient)
{
	//读取消息
	int nLen = pClient->ReadData();
	if (nLen > 0)
		OnNetRecv(pClient);
	return nLen;
}

void CellServer::OnNetRecv(CELLClient* pClient)
{
	//统计接收消息次数
	if (_pNetEvent)
	{
		_pNetEvent->OnNetRecv(pClient);
	}
}

void CellServer::OnNetMsg(CELLClient *pClient, netmsg_DataHeader *pHeader)
{
	if (_pNetEvent)
	{
		_pNetEvent->OnNetMsg(this, pClient, pHeader);
	}
}

inline void CellServer::Close()
{
	CELLLog_Debug("CellServer %d::Close start", _id);
	_cellSendServer.Close();
	_thread.Close();
	CELLLog_Debug("CellServer %d::Close end", _id);
}

inline void CellServer::CleanClients()
{
	//关闭所有的客户端连接
	for (size_t n = 0; n < _clients.size(); n++)
	{
		delete _clients[n];
	}
	_clients.clear();

	for (size_t n = 0; n < _clientsBuf.size(); n++)
	{
		delete _clientsBuf[n];
	}
	_clientsBuf.clear();
}
#endif // _CELL_SERVER_HPP_
