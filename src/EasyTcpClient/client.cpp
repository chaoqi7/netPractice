
#include <thread>

#include "NetMsg.h"
#include "EasyTcpClient.hpp"

void cmdThread(EasyTcpClient* pClient)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("cmdThread need exit.\n");
			pClient->Close();
			break;
		}
		else if (0 == strcmp(cmdBuf, "login")) {
			Login login = {};
			strcpy(login.userName, "chaoqi");
			strcpy(login.passWord, "chaoqimima");
			pClient->SendData(&login);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout = {};
			strcpy(logout.userName, "chaoqi");
			pClient->SendData(&logout);
		}
		else {
			printf("unknown command, input again.\n");
		}
	}	
}

int main(int argc, char** argv)
{
	EasyTcpClient client;
	client.Connect("192.168.3.248", 4567);

	std::thread t1(cmdThread, &client);
	t1.detach();

	Login login = {};
	strcpy(login.userName, "chaoqi");
	strcpy(login.passWord, "chaoqimima");
	//pClient->SendData(&login);

	while (client.IsRun())
	{
		client.OnRun();
		client.SendData(&login);
	}

	printf("客户端也退出.\n");

	client.Close();

	getchar();

	return 0;
}


