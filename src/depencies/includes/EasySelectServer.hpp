#ifndef _EASY_SELECT_SERVER_HPP_
#define _EASY_SELECT_SERVER_HPP_

#include "EasyTcpServer.hpp"
#include "CELLFDSet.hpp"
#include "CELLSelectServer.hpp"

class EasySelectServer : public EasyTcpServer
{
public:
	~EasySelectServer()
	{
		Close();
	}
	void Start(int cellServerCount = 1)
	{
		EasyTcpServer::Start<CELLSelectServer>(cellServerCount);
	}
protected:

	//工作函数
	void OnRun(CELLThread* pThread)
	{
		CELLFDSet fdRead;
		SOCKET curfd = socketfd();
		while (pThread->IsRun())
		{
			time4msg();
			fdRead.zero();
			//把服务器 socket 加入监听
			fdRead.add(curfd);
			/*
			NULL:一直阻塞
			timeval 只能精确到秒
			*/
			timeval t = { 0, 1 };
			int ret = select((int)curfd + 1, fdRead.fdset(), nullptr, nullptr, &t);
			if (SOCKET_ERROR == ret)
			{
				CELLLog_PError("EasySelectServer::OnRun select.");
				pThread->Exit();
				break;
			}
			else if (0 == ret) {
				//CELLLog_Info("select timeout.");
				//continue;
			}

			if (fdRead.has(curfd))
			{
				Accept();
			}
		}
	}
};

#endif // !_EASY_SELECT_SERVER_HPP_
