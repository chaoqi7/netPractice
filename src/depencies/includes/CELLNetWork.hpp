#ifndef _CELL_NET_WORK_H_
#define _CELL_NET_WORK_H_

#include "CELL.hpp"

class CELLNetWork
{
private:
	CELLNetWork()
	{
#ifdef _WIN32
		//初始化 socket 2.0 环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#else
		//忽略异常信号，默认会导致进程终止
		signal(SIGPIPE, SIG_IGN);
#endif
	}
	~CELLNetWork()
	{
#ifdef _WIN32
		WSACleanup();
#endif
	}
public:
	static void Init()
	{
		static CELLNetWork obj;
	}

	static void close(SOCKET s)
	{
#ifdef _WIN32
		//断开 socket 连接
		closesocket(s);
#else
		//断开 socket 连接
		close(s);
#endif
		s = INVALID_SOCKET;
	}
};


#endif // _CELL_NET_WORK_H_
