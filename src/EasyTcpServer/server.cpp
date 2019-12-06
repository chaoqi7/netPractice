
#include "EasyTcpServer.hpp"
#include <thread>
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

class MyServer : public EasyTcpServer
{
public:
	void OnNetJoin(ClientSocket* pClient) override
	{
		_clientCount++;
	}

	void OnNetLeave(ClientSocket* pClient) override
	{
		_clientCount--;
	}

	void OnNetMsg(ClientSocket* pClient, DataHeader* pHeader) override
	{
		_msgCount++;
		switch (pHeader->cmd)
		{
		case CMD_LOGIN:
		{
			Login* pLogin = (Login*)pHeader;
			//printf("收到命令:CMD_LOGIN, 数据长度:%d, userName:%s, password:%s\n",
			//	pLogin->dataLength, pLogin->userName, pLogin->passWord);
			//忽略登录消息的具体数据
			LoginResult loginResult;
			pClient->SendData(&loginResult);
		}
		break;
		case CMD_LOGINOUT:
		{
			Logout* pLogout = (Logout*)pHeader;
			//printf("收到命令:CMD_LOGINOUT, 数据长度:%d, userName:%s\n",
			//	pLogout->dataLength, pLogout->userName);
			//忽略登出消息的具体数据
			//LogoutResult loginoutResult;
			//pClient->SendData(&loginoutResult);
		}
		break;
		default:
		{
			printf("收到未定义消息.\n");
			DataHeader dh;
			pClient->SendData(&dh);
		}
		break;
		}
	}
private:

};


int main(int argc, char** argv)
{
	MyServer server;
	server.Bind(nullptr, 4567);
	server.Listen(128);
	server.Start(4);

	std::thread t1(cmdThread);
	t1.detach();

	while (g_bRun)
	{
		server.OnRun();
	}

	printf("任务结束.\n");
	
	server.Close();
	getchar();

	return 0;
}