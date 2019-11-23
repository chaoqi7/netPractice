
#ifdef _WIN32
//windows
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
//linux and osx
#endif // _WIN32

#include <stdio.h>

#include "NetMsg.h"



int main(int argc, char** argv)
{
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat;
	WSAStartup(ver, &dat);

	//建立socket
	SOCKET _sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//绑定网络端口
	sockaddr_in _sAddr = {};
	_sAddr.sin_family = AF_INET;
	_sAddr.sin_port = htons(4567);
	_sAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (SOCKET_ERROR == bind(_sock, (sockaddr*)&_sAddr, sizeof(_sAddr)))
	{
		printf("绑定网络端口错误.\n");
		return -1;
	}
	else {
		printf("绑定网络端口成功.\n");
	}

	//监听
	if (SOCKET_ERROR == listen(_sock, 64))
	{
		printf("监听网络端口错误.\n");
		return -1;
	}
	else {
		printf("监听网络端口成功.\n");
	}

	//接受新客户端连接
	sockaddr_in _cAddr = {};
	int _cAddrLen = sizeof(sockaddr_in);
	SOCKET _cSock = accept(_sock, (sockaddr*)&_cAddr, &_cAddrLen);
	if (INVALID_SOCKET == _cSock)
	{
		printf("接受到无效客户端SOCKET\n");
		return -1;
	}

	printf("接收到新客户端<sock:%d>: IP = %s\n", (int)_cSock, inet_ntoa(_cAddr.sin_addr));

	while (true)
	{
		DataHeader dh = {};
		int nLen = recv(_cSock, (char*)&dh, sizeof(DataHeader), 0);
		if (nLen <= 0)
		{
			printf("nLen=%d 客户端也退出，任务结束.\n", nLen);
			break;
		}

		switch (dh.cmd)
		{
		case CMD_LOGIN:
		{
			Login login = {};
			recv(_cSock, (char*)&login+sizeof(DataHeader), sizeof(Login)-sizeof(DataHeader), 0);
			printf("收到命令:CMD_LOGIN, 数据长度:%d, userName:%s, password:%s\n", 
				login.dataLength, login.userName, login.passWord);
			//忽略登录消息的具体数据
			LoginResult loginResult;
			send(_cSock, (const char*)&loginResult, sizeof(LoginResult), 0);
		}
		break;
		case CMD_LOGINOUT:
		{
			Logout logout = {};
			recv(_cSock, (char*)&logout + sizeof(DataHeader), sizeof(Logout)-sizeof(DataHeader), 0);
			printf("收到命令:CMD_LOGINOUT, 数据长度:%d, userName:%s\n",
				logout.dataLength, logout.userName);
			//忽略登出消息的具体数据
			LogoutResult loginoutResult;
			send(_cSock, (const char*)&loginoutResult, sizeof(LogoutResult), 0);
		}
		break;
		default:
		{
			DataHeader dh;
			send(_cSock, (const char*)&dh, sizeof(DataHeader), 0);
		}
		break;
		}
	}

	printf("任务结束.\n");
	//断开 socket 连接
	closesocket(_sock);


	WSACleanup();

	getchar();

	return 0;
}