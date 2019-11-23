
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

	while (true)
	{
		//输入请求命令
		char cmdBuf[128] = {};
		scanf("%s", cmdBuf);	
		//处理请求
		if (0 == strcmp(cmdBuf, "exit"))
		{
			break;
		} 
		else if (0 == strcmp(cmdBuf, "login")) {					
			Login login;
			strcpy(login.userName, "chaoqi");
			strcpy(login.passWord, "mima");
			//向服务器发送指令
			send(_sock, (const char*)&login, sizeof(login), 0);
			//接收服务器返回的指令
			LoginResult loginResult = {};
			recv(_sock, (char*)&loginResult, sizeof(LoginResult), 0);
			printf("返回登录结果: %d.\n", loginResult.result);
		}
		else if (0 == strcmp(cmdBuf, "logout")) {
			Logout logout;
			strcpy(logout.userName, "chaoqi");
			//向服务器发送指令
			send(_sock, (const char*)&logout, sizeof(Logout), 0);
			//接收服务器返回的指令
			LogoutResult logoutResult = {};
			recv(_sock, (char*)&logoutResult, sizeof(LogoutResult), 0);
			printf("返回登出结果: %d.\n", logoutResult.result);
		}
		else {
			printf("不识别的指令，请重新输入.\n");
		}
	}

	printf("客户端也退出.\n");

	closesocket(_sock);
	
	WSACleanup();

	getchar();

	return 0;
}


