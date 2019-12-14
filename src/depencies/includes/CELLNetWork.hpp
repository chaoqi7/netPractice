#ifndef _CELL_NET_WORK_H_
#define _CELL_NET_WORK_H_

#include "CELL.hpp"

class CELLNetWork
{
private:
	CELLNetWork()
	{
#ifdef _WIN32
		//��ʼ�� socket 2.0 ����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#else
		//�����쳣�źţ�Ĭ�ϻᵼ�½�����ֹ
		if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		{
			return;
		}
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
};


#endif // _CELL_NET_WORK_H_
