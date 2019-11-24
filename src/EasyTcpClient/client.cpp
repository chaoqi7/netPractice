
#ifdef _WIN32
//windows
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
//linux and osx
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
typedef int SOCKET
#endif // _WIN32

#include <stdio.h>
#include <thread>

#include "NetMsg.h"

bool g_bRun = true;

int processor(SOCKET cSock)
{
	char msgBuf[1024] = {};
	int nLen = recv(cSock, msgBuf, sizeof(DataHeader), 0);
	if (nLen <= 0)
	{
		printf("nLen=%d <sock=%d>与服务器断开连接，任务结束.\n", nLen, cSock);
		return -1;
	}

	DataHeader* pDh = (DataHeader*)msgBuf;
	switch (pDh->cmd)
	{
	case CMD_LOGIN_RESULT:
	{
		recv(cSock, msgBuf + sizeof(DataHeader), pDh->dataLength - sizeof(DataHeader), 0);
		LoginResult* pLoginResult = (LoginResult*)msgBuf;
		printf("收到服务器返回消息 CMD_LOGIN_RESULT, Result:%d, len:%d\n",
			pLoginResult->result, pLoginResult->dataLength);
	}
	break;
	case CMD_LOGINOUT_RESULT:
	{
		recv(cSock, msgBuf + sizeof(DataHeader), pDh->dataLength - sizeof(DataHeader), 0);
		LogoutResult* pLogoutResult = (LogoutResult*)msgBuf;
		printf("收到服务器返回消息 CMD_LOGINOUT_RESULT, Result:%d, len:%d\n", 
			pLogoutResult->result, pLogoutResult->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		recv(cSock, msgBuf + sizeof(DataHeader), pDh->dataLength - sizeof(DataHeader), 0);
		NewUserJoin* pUserJoin = (NewUserJoin*)msgBuf;
		printf("收到服务器返回消息 CMD_NEW_USER_JOIN, sock:%d, len:%d\n", 
			pUserJoin->sock, pUserJoin->dataLength);
	}
	break;
	default:
		break;
	}
}

void cmdThread(SOCKET sock)
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
			Login login = {};
			strcpy(login.userName, "chaoqi");
			strcpy(login.passWord, "chaoqimima");
			send(sock, (const char*)&login, sizeof(Login), 0);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout = {};
			strcpy(logout.userName, "chaoqi");
			send(sock, (const char*)&logout, sizeof(Logout), 0);
		}
		else {
			printf("unknown command, input again.\n");
		}
	}	
}

int main(int argc, char** argv)
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat = {};
	WSAStartup(ver, &dat);

	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		printf("create socket fail.\n");
		return -1;
	}
	else {
		printf("创建 socket 成功.\n");
	}

	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(4567);
	_sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("连接服务器失败.\n");
		return -1;
	}
	else {
		printf("连接服务器成功.\n");
	}

	std::thread cmdThread(cmdThread, _sock);
	cmdThread.detach();

	while (g_bRun)
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 1, 0 };
		int ret = select(_sock, &fdReads, nullptr, nullptr, &t);
		if (ret == SOCKET_ERROR)
		{
			printf("select error.\n");
			break;
		}
		else if (ret == 0) {
			//printf("select time out.\n");
			//continue;
		}

		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == processor(_sock))
			{
				printf("select processor error.\n");
				break;
			}
		}
	}

	printf("客户端也退出.\n");

	closesocket(_sock);
	
	WSACleanup();

	getchar();

	return 0;
}


