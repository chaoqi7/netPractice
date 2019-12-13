
#include <thread>
#include <atomic>

#include "NetMsg.h"
#include "EasyTcpClient.hpp"
#include "CELLTimeStamp.hpp"

const int g_cCount = 10000;
const int g_tCount = 4;
bool g_bRun = true;


std::atomic<int> g_sendCount(0);
std::atomic<int> g_readyCount(0);

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
		client[n]->Connect("127.0.0.1", 4567);
		//printf("Connect=%d\n", n);
	}

	g_readyCount++;
	if (g_readyCount < g_tCount)
	{
		std::chrono::milliseconds t(1);
		std::this_thread::sleep_for(t);
	}

	netmsg_C2S_Login login[1] = {};
	for (int n = 0; n < 1; n++)
	{
		strcpy(login[n].userName, "chaoqi");
		strcpy(login[n].passWord, "chaoqimima");
	}

	//pClient->SendData(&login);
	CELLTimeStamp tTime;
	auto oldTime = tTime.getElapseTimeInSeconds();
	while (g_bRun)
	{
		for (int n = begin; n < end; n++)
		{
			if (SOCKET_ERROR != client[n]->SendData((const char*)&login, sizeof(login)))
				g_sendCount++;
// 			if (tTime.getElapseTimeInSeconds() - oldTime > 3.0 && n == begin)
// 			{
// 				continue;
// 			}
			client[n]->OnRun();
		}

		//std::chrono::milliseconds t(5000);
		//std::this_thread::sleep_for(t);
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
	CELLTimeStamp tTime;
	while (g_bRun)
	{
		auto t = tTime.getElapseTimeInSeconds();
		if (t >= 1.0)
		{
			printf("thread<%d>, clients<%d>, time<%lf>, send<%d>\n",
				(int)g_tCount, (int)g_cCount, t, (int)g_sendCount);
			g_sendCount = 0;
			tTime.update();
		}
	}

	printf("exit the Main Thread.\n");
	return 0;
}


