
#include <stdio.h>
#include "CELLWriteStream.hpp"
#include "CELLReadStream.hpp"
#include "NetMsg.h"
#include "EasyTcpClient.hpp"

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
		case CMD_S2C_STREAM:
		{
			CELLReadStream r(pHeader);
			auto a2 = r.ReadNetLength();
			auto a1 = r.ReadNetCMD();

			auto a3 = r.ReadInt8();
			auto a4 = r.ReadInt16();
			auto a5 = r.ReadInt32();
			auto a6 = r.ReadInt64();
			auto a7 = r.ReadUInt16();

			char a8[128] = {};
			auto a9 = r.ReadArray(a8, 128);
			char a10[128] = {};
			auto a11 = r.ReadArray(a10, 128);
			int a12[128] = {};
			auto a13 = r.ReadArray(a12, 128);
			auto a14 = r.ReadFloat();
			auto a15 = r.ReadDouble();
		}
		break;
		default:
			CELLLog::Info("收到未定义消息.\n");
			break;
		}
	}
};

int main(int argc, char** argv)
{
	CELLWriteStream w;
	w.WriteNetCMD(CMD_C2S_STREAM);
	w.WriteInt8(1);
	w.WriteInt16(2);
	w.WriteInt32(3);
	w.WriteInt64(4);
	w.WriteUInt16(5);
	w.WriteFloat(6.7f);
	w.WriteDouble(8.9);

	w.WriteString("client");
	char* arr1 = "haha";
	w.WriteArray(arr1, (uint32_t)strlen(arr1));
	int arr2[] = { 10, 11, 12, 13, 15 };
	w.WriteArray(arr2, 5);
	w.Finish();

	MyClient my;
	my.Connect("127.0.0.1", 4567);
	my.SendData(&w);
	while (my.IsRun())
	{
		my.OnRun();

	}

	//CELLReadStream r = 

	getchar();
}