
#include <thread>

#include "NetMsg.h"
#include "EasyTcpClient.hpp"

const int g_CCount = 2000;
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
		else if (0 == strcmp(cmdBuf, "login")) {
// 			Login login = {};
// 			strcpy(login.userName, "chaoqi");
// 			strcpy(login.passWord, "chaoqimima");
// 			pClient->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
// 			Logout logout = {};
// 			strcpy(logout.userName, "chaoqi");
// 			pClient->SendData(&logout);
		}
		else {
			printf("unknown command, input again.\n");
		}
	}	
}

int main(int argc, char** argv)
{
	EasyTcpClient* client[g_CCount];
	for (int n = 0; n < g_CCount; n++)
	{
		if (!g_bRun)
		{
			return 0;
		}
		client[n] = new EasyTcpClient();
		//client[n]->Connect("192.168.3.248", 4567);
	}

	for (int n = 0; n < g_CCount; n++)
	{
		if (!g_bRun)
		{
			return 0;
		}
		//client[n] = new EasyTcpClient();
		client[n]->Connect("192.168.3.248", 4567);
		printf("Connect=%d\n", n);
	}

	std::thread t1(cmdThread);
	t1.detach();

	Login login = {};
	strcpy(login.userName, "chaoqi");
	strcpy(login.passWord, "chaoqimima");
	//pClient->SendData(&login);

	while (g_bRun)
	{
		for (int n = 0; n < g_CCount; n++)
		{
			client[n]->SendData(&login);
			//client[n]->OnRun();			
		}
	}

	printf("客户端也退出.\n");
	for (int n = 0; n < g_CCount; n++)
	{
		client[n]->Close();
	}

	return 0;
}


