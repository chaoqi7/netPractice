
#include "NetMsg.h"
#include "EasyIOCPClient.hpp"
#include "CELLTimeStamp.hpp"
#include "CELLConfig.hpp"

#include <thread>
#include <atomic>
#include <vector>
//服务器IP地址
const char *strIP = "127.0.0.1";
//服务器端口
uint16_t nPort = 4567;
//发送线程数量
int nThread = 1;
//客户端数量
int nClient = 1;
//客户端每次发几条消息
int nMsg = 1;
//写入消息到缓冲区的间隔时间
int nSendSleep = 10;
//工作休眠时间
int nWorkSleep = 1;
//客户端发送缓冲区大小
int nSendBuffSize = SEND_BUF_SIZE;
//客户端接收缓冲区大小
int nRecvBuffSize = RECV_BUF_SIZE;

std::atomic<int> g_sendCount(0);
std::atomic<int> g_readyCount(0);
std::atomic<int> g_nConnect(0);

class MyClient : public EasyIOCPClient
{
public:
	MyClient()
	{
		_bCheckMsgID = CELLConfig::Instance().hasKey("-checkMsgID");
	}

	void OnNetMsg(netmsg_DataHeader *pHeader) override
	{
		switch (pHeader->cmd)
		{
		case CMD_S2C_LOGIN:
		{
			netmsg_LoginR *login = (netmsg_LoginR *)pHeader;
			if (_bCheckMsgID)
			{
				if (login->msgID != _nRecvMsgID)
				{
					CELLLog_Error("OnNetMsg socket<%d> msgID<%d> _nRecvMsgID<%d> %d",
								  (int)_pClient->socketfd(), login->msgID, _nRecvMsgID, login->msgID - _nRecvMsgID);
				}
				_nRecvMsgID++;
			}
		}
		break;
		case CMD_S2C_LOGOUT:
		{
			netmsg_LogoutR *pLogoutResult = (netmsg_LogoutR *)pHeader;
			//CELLLog_Info("<sockt=%d>收到服务器返回消息 CMD_LOGINOUT_RESULT, Result:%d, len:%d",
			//	(int)_sock, pLogoutResult->result, pLogoutResult->dataLength);
		}
		break;
		case CMD_S2C_NEW_USER_JOIN:
		{
			netmsg_NewUserJoin *pUserJoin = (netmsg_NewUserJoin *)pHeader;
			//CELLLog_Info("<sockt=%d>收到服务器返回消息 CMD_NEW_USER_JOIN, sock:%d, len:%d",
			//	(int)_sock, pUserJoin->sock, pUserJoin->dataLength);
		}
		break;
		case CMD_S2C_ERROR:
		{
			CELLLog_Error("CMD_ERROR...");
		}
		break;
		default:
			CELLLog_Error("收到未定义消息.");
			break;
		}
	}

	int sendTest(netmsg_Login *login)
	{
		int ret = 0;
		if (_nSendCount > 0)
		{
			login->msgID = _nSendMsgID;
			ret = SendData(login);
			if (SOCKET_ERROR != ret)
			{
				++_nSendMsgID;
				--_nSendCount;
			}
		}
		return ret;
	}

	bool checkSend(time_t dt)
	{
		_tResetTime += dt;
		if (_tResetTime >= nSendSleep)
		{
			_tResetTime -= nSendSleep;
			_nSendCount = nMsg;
		}
		return _nSendCount > 0;
	}

private:
	//接收消息ID计数
	int _nRecvMsgID = 1;
	//发送消息ID计数
	int _nSendMsgID = 1;
	//发送时间计数
	time_t _tResetTime = 0;
	//发送条数计数
	int _nSendCount = 1;

	bool _bCheckMsgID = 0;
};

void workThread(CELLThread *pThread, int id)
{
	std::vector<MyClient *> clients(nClient);
	int begin = 0;
	int end = nClient;

	CELLLog_Info("workThread thread<%d>,nClient<%d> start", id, nClient);
	for (int n = begin; n < end; n++)
	{
		if (!pThread->IsRun())
			break;
		clients[n] = new MyClient();
		//多线程让一下 CPU
		CELLThread::Sleep(0);
	}

	for (int n = begin; n < end; n++)
	{
		if (!pThread->IsRun())
			break;
		if (INVALID_SOCKET == clients[n]->InitSocket(nSendBuffSize, nRecvBuffSize))
			break;
		if (SOCKET_ERROR == clients[n]->Connect(strIP, nPort))
			break;
		g_nConnect++;
		CELLThread::Sleep(0);
	}

	CELLLog_Info("workThread thread<%d>, Connect<begin=%d, end=%d, g_nConnect=%d>",
				 id, begin, end, (int)g_nConnect);

	//所有连接完成
	g_readyCount++;
	if (g_readyCount < nThread && pThread->IsRun())
	{
		//等待其它线程准备好，再发送数据
		CELLThread::Sleep(10);
	}

	netmsg_Login login;
	strcpy(login.userName, "chaoqi");
	strcpy(login.passWord, "cqmima");

	auto t2 = CELLTime::getNowInMilliseconds();
	auto t0 = t2;
	auto dt = t0;
	CELLTimeStamp tTime;
	while (pThread->IsRun())
	{
		t0 = CELLTime::getNowInMilliseconds();
		dt = t0 - t2;
		t2 = t0;

		int count = 0;
		//每轮每个客户端发送 nMsg 条数据
		for (int m = 0; m < nMsg; m++)
		{
			//每个客户端每次发送一条消息
			for (int n = begin; n < end; n++)
			{
				if (clients[n]->IsRun())
				{
					if (clients[n]->sendTest(&login) > 0)
					{
						g_sendCount++;
					}
				}
			}
		}

		for (int n = begin; n < end; n++)
		{
			if (clients[n]->IsRun())
			{
				if (!clients[n]->OnRun(0))
				{
					//断开连接
					g_nConnect--;
					continue;
				}
				//检测发送计数是否需要重置
				clients[n]->checkSend(dt);
			}
		}
	}
	CELLThread::Sleep(nWorkSleep);

	for (int n = begin; n < end; n++)
	{
		clients[n]->Close();
		delete clients[n];
	}
	CELLLog_Info("thread<%d>, exit", id);
	--g_readyCount;
}

int main(int argc, char **argv)
{
	CELLLog::setLogPath("clientLog", "w", false);

	CELLConfig::Instance().Init(argc, argv);

	strIP = CELLConfig::Instance().getStr("strIP", "127.0.0.1");
	nPort = (uint16_t)CELLConfig::Instance().getInt("nPort", nPort);
	nThread = CELLConfig::Instance().getInt("nThread", nThread);
	nClient = CELLConfig::Instance().getInt("nClient", nClient);
	nMsg = CELLConfig::Instance().getInt("nMsg", nMsg);
	nSendSleep = CELLConfig::Instance().getInt("nSendSleep", nSendSleep);
	nSendBuffSize = CELLConfig::Instance().getInt("nSendBuffSize", SEND_BUF_SIZE);
	nRecvBuffSize = CELLConfig::Instance().getInt("nRecvBuffSize", RECV_BUF_SIZE);
	//UI 线程
	CELLThread tCmd;
	tCmd.Start(nullptr, [](CELLThread *pThread) {
		while (pThread->IsRun())
		{
			char cmdBuf[256] = {};
			scanf("%s", cmdBuf);
			if (0 == strcmp(cmdBuf, "exit"))
			{
				pThread->Exit();
				CELLLog_Info("退出 cmdThread 线程");
				break;
			}
			else
			{
				CELLLog_Info("不支持的命令");
			}
		}
	});

	std::vector<CELLThread *> threads;
	for (int n = 0; n < nThread; n++)
	{
		CELLThread *t = new CELLThread();
		t->Start(nullptr, [n](CELLThread *pThread) {
			workThread(pThread, n + 1);
		});
		threads.push_back(t);
	}

	CELLTimeStamp tTime;
	while (tCmd.IsRun())
	{
		auto t = tTime.getElapseTimeInSeconds();
		if (t >= 1.0)
		{
			CELLLog_Info("thread<%d>, clients<%d>, g_nConnect<%d>, time<%lf>, send<%d>",
						 nThread, nClient, (int)g_nConnect, t, (int)g_sendCount);
			g_sendCount = 0;
			tTime.update();
		}
		CELLThread::Sleep(1);
	}

	for (auto t : threads)
	{
		t->Close();
		delete t;
	}

	CELLLog_Info("exit the Main Thread.");
	return 0;
}
