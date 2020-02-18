#ifndef _EASY_EPOLL_SERVER_HPP_
#define _EASY_EPOLL_SERVER_HPP_

#ifdef __linux__

#include "EasyTcpServer.hpp"
#include "CELLEpoll.hpp"
#include "CELLEpollServer.hpp"

class EasyEpollServer : public EasyTcpServer
{
public:
	~EasyEpollServer()
	{
		Close();
	}
	void Start(int cellServerCount = 1)
	{
		EasyTcpServer::Start<CELLEpollServer>(cellServerCount);
	}

protected:
	//工作函数
	void OnRun(CELLThread *pThread)
	{
		CELLEpoll ep;
		ep.create(1);
		ep.ctl(EPOLL_CTL_ADD, socketfd(), EPOLLIN);
		while (pThread->IsRun())
		{
			time4msg();

			int ret = ep.wait(1);
			if (ret < 0)
			{
				CELLLog_PError("EasyEpollServer::OnRun wait.");
				pThread->Exit();
				break;
			}
			else
			{
				auto events = ep.events();
				for (int n = 0; n < ret; n++)
				{
					if (events[n].data.fd == socketfd())
					{
						Accept();
					}
				}
			}
		}
	}
};

#endif // __linux__

#endif // !_EASY_EPOLL_SERVER_HPP_
