
#include "EasyTcpServer.hpp"
#include "CELLLog.hpp"

class MyServer : public EasyTcpServer
{
public:
	MyServer(int nSendSize, int nRecvSize)
		:EasyTcpServer(nSendSize, nRecvSize)
	{
	}
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
			pClient->ResetDTHeart();
			netmsg_C2S_Login* pLogin = (netmsg_C2S_Login*)pHeader;
			//CELLLog::Info("收到命令:CMD_LOGIN, 数据长度:%d, userName:%s, password:%s\n",
			//	pLogin->dataLength, pLogin->userName, pLogin->passWord);
			//忽略登录消息的具体数据
			//netmsg_S2C_Login* ret = new netmsg_S2C_Login();
			//pCellServer->AddSendTask(pClient, ret);
			netmsg_S2C_Login ret;
			if (SOCKET_ERROR == pClient->SendData(&ret))
			{
				CELLLog::Info("send buf full.\n");
			}
		}
		break;
		case CMD_LOGOUT:
		{
			netmsg_C2S_Logout* pLogout = (netmsg_C2S_Logout*)pHeader;
			//CELLLog::Info("收到命令:CMD_LOGINOUT, 数据长度:%d, userName:%s\n",
			//	pLogout->dataLength, pLogout->userName);
			//忽略登出消息的具体数据
			//netmsg_S2C_Logout* ret = new netmsg_S2C_Logout();
			//pCellServer->AddSendTask(pClient, ret);
		}
		break;
		case CMD_C2S_HEART:
		{
			pClient->ResetDTHeart();
			netmsg_S2C_Heart* ret = new netmsg_S2C_Heart();
			pCellServer->AddSendTask(pClient, ret);
			//netmsg_S2C_Heart ret;
			//pClient->SendData(&ret);
		}
		break;
		default:
		{
			CELLLog::Info("收到未定义消息.\n");
			netmsg_DataHeader* ret = new netmsg_DataHeader();
			pCellServer->AddSendTask(pClient, ret);
			//netmsg_DataHeader ret;
			//pClient->SendData(&ret);
		}
		break;
		}
	}
private:

};


int main(int argc, char** argv)
{
	CELLLog::setLogPath("serverlog.txt", "w");
	MyServer server(SEND_BUF_SIZE, RECV_BUF_SIZE);
	server.Bind(nullptr, 4567);
	server.Listen(128);
	server.Start(4);

	MyServer server2(SEND_BUF_SIZE, RECV_BUF_SIZE);
	server2.Bind(nullptr, 4568);
	server2.Listen(128);
	server2.Start(4);

	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			CELLLog::Info("cmdThread need exit.\n");
			break;
		}
		else {
			CELLLog::Info("unknown command, input again.\n");
		}
	}

// 	CellTaskServer task;
// 	task.Start(1);
// 	std::chrono::milliseconds t(1000);
// 	std::this_thread::sleep_for(t);
// 	task.Close();

	CELLLog::Info("任务结束.\n");
	return 0;
}

