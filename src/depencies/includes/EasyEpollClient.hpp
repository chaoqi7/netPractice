#ifndef _EASY_EPOLL_CLIENT_HPP_
#define _EASY_EPOLL_CLIENT_HPP_

#ifdef __linux__

#include "EasyTcpClient.hpp"
#include "CELLEpoll.hpp"

class EasyEpollClient : public EasyTcpClient
{
public:
	//循环执行任务（当前使用select)
	bool OnRun(int microseconds);
	void OnInitSocketComplete() override
	{
		_ep.create(1);
		_ep.ctl(EPOLL_CTL_ADD, _pClient, EPOLLIN);
	}

private:
	CELLEpoll _ep;
};

inline bool EasyEpollClient::OnRun(int microseconds)
{
	if (!IsRun())
	{
		return false;
	}

	if (_pClient->NeedWrite())
	{
		_ep.ctl(EPOLL_CTL_MOD, _pClient, EPOLLIN | EPOLLOUT);
	}
	else
	{
		_ep.ctl(EPOLL_CTL_MOD, _pClient, EPOLLIN | EPOLLOUT);
	}

	int ret = _ep.wait(microseconds);
	if (ret == EPOLL_ERROR)
	{
		CELLLog_PError("EasyEpollClient::OnRun wait.");
		Close();
		return false;
	}
	else
	{
		auto events = _ep.events();
		for (int i = 0; i < ret; i++)
		{
			if (events[i].events & EPOLLIN)
			{
				if (-1 == RecvData())
				{
					//CELLLog_PError("EasyEpollClient::OnRun RecvData.");
					Close();
					return false;
				}
			}

			if (events[i].events & EPOLLOUT)
			{
				if (-1 == _pClient->SendDataReal())
				{
					CELLLog_PError("EasyEpollClient::OnRun SendDataReal.");
					Close();
					return false;
				}
			}
		}
	}
	return true;
}

#endif // __linux__

#endif //_EASY_EPOLL_CLIENT_HPP_
