
#include <thread>

#include "NetMsg.h"
#include "EasyTcpClient.hpp"

const int g_cCount = 4000;
const int g_tCount = 4;
bool g_bRun = true;


void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("cmdThread need exit.\n");
			g_bRun = false;
			break;
		}
		else {
			printf("unknown command, input again.\n");
		}
	}	
}

void sendThread(int id)
{
	EasyTcpClient* client[g_cCount];
	int cNum = g_cCount / g_tCount;
	int begin = (id - 1)*cNum;
	int end = id * cNum;

	printf("sendThread id=%d, begin=%d, end=%d\n", id, begin, end);

	for (int n = begin; n < end; n++)
	{
		client[n] = new EasyTcpClient();
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("192.168.3.248", 4567);
		//printf("Connect=%d\n", n);
	}

	Login login[10] = {};
	for (int n = 0; n < 10; n++)
	{
		strcpy(login[n].userName, "chaoqi");
		strcpy(login[n].passWord, "chaoqimima");
	}

	//pClient->SendData(&login);

	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			client[n]->SendData((const char*)&login, sizeof(login));
			//client[n]->OnRun();			
		}
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Close();
	}
}

int main(int argc, char** argv)
{
	//UI 线程
	std::thread t1(cmdThread);
	t1.detach();

	for (int n = 0; n < g_tCount; n++)
	{
		std::thread t(sendThread, n + 1);
		t.detach();
	}

	getchar();
	printf("exit the Main Thread.\n");
	return 0;
}


