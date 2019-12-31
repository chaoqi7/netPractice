
#include <thread>
#include <atomic>

#include "NetMsg.h"
#include "EasyTcpClient.hpp"
#include "CELLTimeStamp.hpp"

const int g_cCount = 1000;
const int g_tCount = 4;
bool g_bRun = true;


std::atomic<int> g_sendCount(0);
std::atomic<int> g_readyCount(0);

class MyClient : public EasyTcpClient
{
public:
	void OnNetMsg(netmsg_DataHeader* pHeader) override 
	{
		switch (pHeader->cmd)
		{
		case CMD_S2C_LOGIN:
		{
			netmsg_S2C_Login* pLoginResult = (netmsg_S2C_Login*)pHeader;
			//CELLLog::Info("<sockt=%d>收到服务器返回消息 CMD_LOGIN_RESULT, Result:%d, len:%d\n",
			//	(int)_sock, pLoginResult->result, pLoginResult->dataLength);
		}
		break;
		case CMD_S2C_LOGOUT:
		{
			netmsg_S2C_Logout* pLogoutResult = (netmsg_S2C_Logout*)pHeader;
			//CELLLog::Info("<sockt=%d>收到服务器返回消息 CMD_LOGINOUT_RESULT, Result:%d, len:%d\n",
			//	(int)_sock, pLogoutResult->result, pLogoutResult->dataLength);
		}
		break;
		case CMD_S2C_NEW_USER_JOIN:
		{
			netmsg_S2C_NewUserJoin* pUserJoin = (netmsg_S2C_NewUserJoin*)pHeader;
			//CELLLog::Info("<sockt=%d>收到服务器返回消息 CMD_NEW_USER_JOIN, sock:%d, len:%d\n",
			//	(int)_sock, pUserJoin->sock, pUserJoin->dataLength);
		}
		break;
		case CMD_S2C_ERROR:
		{
			CELLLog::Info("CMD_ERROR...\n");
		}
		break;
		default:
			CELLLog::Info("收到未定义消息.\n");
			break;
		}
	}
};


void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			CELLLog::Info("cmdThread need exit.\n");
			g_bRun = false;
			break;
		}
		else {
			CELLLog::Info("unknown command, input again.\n");
		}
	}	
}

void sendThread(int id)
{
	EasyTcpClient* client[g_cCount];
	int cNum = g_cCount / g_tCount;
	int begin = (id - 1)*cNum;
	int end = id * cNum;

	CELLLog::Info("sendThread id=%d, begin=%d, end=%d\n", id, begin, end);

	for (int n = begin; n < end; n++)
	{
		client[n] = new MyClient();
	}

	for (int n = begin; n < end; n++)
	{
		client[n]->Connect("192.168.3.248", 4567);
		//CELLLog::Info("Connect=%d\n", n);
	}

	g_readyCount++;
	if (g_readyCount < g_tCount)
	{
		std::chrono::milliseconds t(1);
		std::this_thread::sleep_for(t);
	}

	const int msgNum = 1;
	netmsg_C2S_Login login[msgNum] = {};
	for (int n = 0; n < msgNum; n++)
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
			for (int k = 0; k < msgNum; k++)
			{
				if (SOCKET_ERROR != client[n]->SendData(&login[k]))
				{
				}
			}
			g_sendCount++;
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
	CELLLog::setLogPath("clientlog.txt", "w");
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
			CELLLog::Info("thread<%d>, clients<%d>, time<%lf>, send<%d>\n",
				(int)g_tCount, (int)g_cCount, t, (int)g_sendCount);
			g_sendCount = 0;
			tTime.update();
		}
	}

	CELLLog::Info("exit the Main Thread.\n");
	return 0;
}


