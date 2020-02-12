#ifndef _CELL_EPOLL_SERVER_HPP_
#define _CELL_EPOLL_SERVER_HPP_

#include <algorithm>

#include "CELLEpoll.hpp"
#include "CELLServer.hpp"

class CELLEpollServer : public CellServer
{
public:
	CELLEpollServer()
	{
		_ep.create(4048);
	}
	~CELLEpollServer()
	{
		Close();
	}
	//通过select 检测可写，可读。
	bool DoNetEvents();
	void OnClientJoin(CELLClient *pClient) override
	{
		//
		_ep.ctl(EPOLL_CTL_ADD, pClient, EPOLLIN);
		CellServer::OnClientJoin(pClient);
	}

	void rmClient(CELLClient *pClient)
	{
		auto iter = std::find(_clients.begin(), _clients.end(), pClient);
		if (iter != _clients.end())
		{
			_clients.erase(iter);
		}
		OnClientLeave(pClient);
	}

private:
	CELLEpoll _ep;
};

inline bool CELLEpollServer::DoNetEvents()
{
	//检查需要可写数据到客户端
	for (size_t n = 0; n < _clients.size(); n++)
	{
		if (_clients[n]->NeedWrite())
		{
			_ep.ctl(EPOLL_CTL_MOD, _clients[n], EPOLLIN | EPOLLOUT);
		}
		else
		{
			_ep.ctl(EPOLL_CTL_MOD, _clients[n], EPOLLIN);
		}
	}

	int ret = _ep.wait(1);
	if (ret < 0)
	{
		CELLLog_PError("CellEpollServer::DoNetEvents wait.");
		return false;
	}
	else if (0 == ret)
	{
		return true;
	}

	auto events = _ep.events();
	for (int n = 0; n < ret; n++)
	{
		CELLClient *pClient = (CELLClient *)events[n].data.ptr;
		if (nullptr == pClient)
		{
			continue;
		}

		//处理可读
		if (events[n].events & EPOLLIN)
		{
			if (SOCKET_ERROR == RecvData(pClient))
			{
				//处理消息出现错误，在全局客户端数据里面删除它.
				rmClient(pClient);
				continue;
			}
		}
		//处理可写
		if (events[n].events & EPOLLOUT)
		{
			if (SOCKET_ERROR == pClient->SendDataReal())
			{
				rmClient(pClient);
			}
		}
	}

	return true;
}

#endif // _CELL_EPOLL_SERVER_HPP_
