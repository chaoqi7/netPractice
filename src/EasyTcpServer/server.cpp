
#include "EasyTcpServer.hpp"
#include <thread>

void cmdThread(EasyTcpServer* pServer)
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			printf("cmdThread need exit.\n");
			pServer->Close();
			break;
		}
		else {
			printf("unknown command, input again.\n");
		}
	}
}

int main(int argc, char** argv)
{
	EasyTcpServer server;
	server.Bind(nullptr, 4567);
	server.Listen(64);

	std::thread t1(cmdThread, &server);
	t1.detach();

	while (server.IsRun())
	{
		server.OnRun();
	}

	printf("任务结束.\n");
	
	server.Close();
	getchar();

	return 0;
}