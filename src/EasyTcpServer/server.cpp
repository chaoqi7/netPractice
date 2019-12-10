
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
	void OnNetJoin(CELLClient* pClient) override
	{
		EasyTcpServer::OnNetJoin(pClient);
	}

	void OnNetLeave(CELLClient* pClient) override
	{
		EasyTcpServer::OnNetLeave(pClient);
	}

	void OnNetMsg(CellServer* pCellServer, CELLClient* pClient, netmsg_DataHeader* pHeader) override
	{
		EasyTcpServer::OnNetMsg(pCellServer, pClient, pHeader);
		switch (pHeader->cmd)
		{
		case CMD_LOGIN:
		{
			netmsg_C2S_Login* pLogin = (netmsg_C2S_Login*)pHeader;
			//printf("收到命令:CMD_LOGIN, 数据长度:%d, userName:%s, password:%s\n",
			//	pLogin->dataLength, pLogin->userName, pLogin->passWord);
			//忽略登录消息的具体数据
			netmsg_S2C_Login* loginResult = new netmsg_S2C_Login();
			pCellServer->AddSendTask(pClient, loginResult);
		}
		break;
		case CMD_LOGOUT:
		{
			netmsg_C2S_Logout* pLogout = (netmsg_C2S_Logout*)pHeader;
			//printf("收到命令:CMD_LOGINOUT, 数据长度:%d, userName:%s\n",
			//	pLogout->dataLength, pLogout->userName);
			//忽略登出消息的具体数据
			//netmsg_S2C_Logout loginoutResult;
			//pClient->SendData(&loginoutResult);
		}
		break;
		default:
		{
			printf("收到未定义消息.\n");
			netmsg_DataHeader dh;
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