
#include "EasyTcpServer.hpp"
#include "CELLLog.hpp"
#include "CELLReadStream.hpp"
#include "CELLWriteStream.hpp"
#include "CELLConfig.hpp"

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
		case CMD_C2S_LOGIN:
		{
			pClient->ResetDTHeart();
			netmsg_Login* pLogin = (netmsg_Login*)pHeader;
			//CELLLog_Info("收到命令:CMD_LOGIN, 数据长度:%d, userName:%s, password:%s",
			//	pLogin->dataLength, pLogin->userName, pLogin->passWord);
			//忽略登录消息的具体数据
			netmsg_LoginR ret;
			ret.msgID = pLogin->msgID;
			if (SOCKET_ERROR == pClient->SendData(&ret))
			{
				CELLLog_Info("send buf full.");
			}
		}
		break;
		case CMD_C2S_LOGOUT:
		{
 			netmsg_Logout* pLogout = (netmsg_Logout*)pHeader;
// 			//CELLLog_Info("收到命令:CMD_LOGINOUT, 数据长度:%d, userName:%s",
// 			//	pLogout->dataLength, pLogout->userName);
// 			//忽略登出消息的具体数据
			netmsg_LogoutR ret;
			pClient->SendData(&ret);
		}
		break;
		case CMD_C2S_HEART:
		{
			pClient->ResetDTHeart();
			netmsg_HeartR ret;
			pClient->SendData(&ret);
		}
		break;
		case CMD_C2S_STREAM:
		{
			pClient->ResetDTHeart();
			CELLReadStream r(pHeader);
			auto a2 = r.ReadNetLength();
			auto a1 = r.ReadNetCMD();
			auto a3 = r.ReadInt8();
			auto a4 = r.ReadInt16();
			auto a5 = r.ReadInt32();
			auto a6 = r.ReadInt64();
			auto a7 = r.ReadUInt16();
			auto a14 = r.ReadFloat();
			auto a15 = r.ReadDouble();
			char a8[128] = {};
			auto a9 = r.ReadArray(a8, 128);
			char a10[128] = {};
			auto a11 = r.ReadArray(a10, 128);
			int a12[128] = {};
			auto a13 = r.ReadArray(a12, 128);

			CELLWriteStream w;
			w.WriteNetCMD(CMD_S2C_STREAM);
			w.WriteInt8(a3);
			w.WriteInt16(a4);
			w.WriteInt32(a5);
			w.WriteInt64(a6);
			w.WriteUInt16(a7);
			w.WriteString("server");
			w.WriteString(a10);
			w.WriteArray(a12, a13);
			w.WriteFloat(a14);
			w.WriteDouble(a15);
			w.Finish();
			pClient->SendData(w.Data(), w.Length());
		}
		break;
		default:
		{
			CELLLog_Info("收到未定义消息.");
			netmsg_DataHeader ret;
			pClient->SendData(&ret);
		}
		break;
		}
	}
private:

};

int main(int argc, char** argv)
{
	CELLLog::setLogPath("serverlog", "w", false);

	CELLConfig::Instance().Init(argc, argv);

	const char* strIP = CELLConfig::Instance().getStr("strIP", "127.0.0.1");
	uint16_t nPort = (uint16_t)CELLConfig::Instance().getInt("nPort", 4567);
	int nThread = CELLConfig::Instance().getInt("nThread", 1);
	int nClient = CELLConfig::Instance().getInt("nClient", 1);

	if (strcmp(strIP, "any") == 0)
	{
		strIP = nullptr;
	}

	MyServer server(SEND_BUF_SIZE, RECV_BUF_SIZE);
	server.Bind(strIP, nPort);
	server.Listen(128);
	server.Start(nThread);

// 	MyServer server2(SEND_BUF_SIZE, RECV_BUF_SIZE);
// 	server2.Bind(nullptr, 4568);
// 	server2.Listen(128);
// 	server2.Start(4);

	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit"))
		{
			CELLLog_Info("cmdThread need exit.");
			break;
		}
		else {
			CELLLog_Info("unknown command, input again.");
		}
	}

// 	CellTaskServer task;
// 	task.Start(1);
// 	std::chrono::milliseconds t(1000);
// 	std::this_thread::sleep_for(t);
// 	task.Close();

	CELLLog_Info("任务结束.");
	return 0;
}

