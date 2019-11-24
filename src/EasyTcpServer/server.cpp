
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
#include <vector>
#include "NetMsg.h"

std::vector<SOCKET> g_clients;

int processor(SOCKET cSock)
{
	char recvBuf[1024] = {};
	int nLen = recv(cSock, recvBuf, sizeof(DataHeader), 0);
	if (nLen <= 0)
	{
		printf("nLen=%d 客户端<sock=%d>也退出，任务结束.\n", nLen, cSock);
		return -1;
	}
	DataHeader* pDh = (DataHeader*)recvBuf;
	switch (pDh->cmd)
	{
	case CMD_LOGIN:
	{
		recv(cSock, recvBuf + sizeof(DataHeader), pDh->dataLength - sizeof(DataHeader), 0);
		Login* pLogin = (Login*)recvBuf;
		printf("收到命令:CMD_LOGIN, 数据长度:%d, userName:%s, password:%s\n",
			pLogin->dataLength, pLogin->userName, pLogin->passWord);
		//忽略登录消息的具体数据
		LoginResult loginResult;
		send(cSock, (const char*)&loginResult, sizeof(LoginResult), 0);
	}
	break;
	case CMD_LOGINOUT:
	{
		recv(cSock, recvBuf + sizeof(DataHeader), pDh->dataLength - sizeof(DataHeader), 0);
		Logout* pLogout = (Logout*)recvBuf;
		printf("收到命令:CMD_LOGINOUT, 数据长度:%d, userName:%s\n",
			pLogout->dataLength, pLogout->userName);
		//忽略登出消息的具体数据
		LogoutResult loginoutResult;
		send(cSock, (const char*)&loginoutResult, sizeof(LogoutResult), 0);
	}
	break;
	default:
	{
		DataHeader dh;
		send(cSock, (const char*)&dh, sizeof(DataHeader), 0);
	}
	break;
	}

	return 0;
}

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

	while (true)
	{
		//nfds 当前 socket 最大值+1（兼容贝克利套接字）. 在 windows 里面可以设置为 0.
		fd_set fdRead;
		fd_set fdWrite;
		fd_set fdExcept;

		FD_ZERO(&fdRead);
		FD_ZERO(&fdWrite);
		FD_ZERO(&fdExcept);
		//把服务器 socket 加入监听
		FD_SET(_sock, &fdRead);
		FD_SET(_sock, &fdWrite);
		FD_SET(_sock, &fdExcept);

		//把全局客户端数据加入可读监听部分
		for (int n = 0; n < g_clients.size(); n++)
		{
			FD_SET(g_clients[n], &fdRead);
		}
		/*
		NULL:一直阻塞
		timeval 只能精确到秒
		*/
		timeval t = { 1, 0 };
		int ret = select(_sock + 1, &fdRead, &fdWrite, &fdExcept, &t);
		if (SOCKET_ERROR == ret)
		{
			printf("select error.\n");
			break;
		}
		else if (0 == ret) {
			//printf("select timeout.\n");
			continue;
		}

		//处理服务器绑定的 socket
		if (FD_ISSET(_sock, &fdRead))
		{
			FD_CLR(_sock, &fdRead);
			//接受新客户端连接
			sockaddr_in _cAddr = {};
			int _cAddrLen = sizeof(sockaddr_in);
			SOCKET cSock = accept(_sock, (sockaddr*)&_cAddr, &_cAddrLen);
			if (INVALID_SOCKET == cSock)
			{
				printf("接受到无效客户端SOCKET\n");
				return -1;
			}
			//把新客户端登录消息广播给所有的客户端
			for (int n = 0; n < g_clients.size(); n++)
			{
				NewUserJoin userJoin;
				userJoin.sock = cSock;
				send(g_clients[n], (const char*)&userJoin, sizeof(NewUserJoin), 0);
			}
			//把新客户端 socket 添加到全局数据里面
			g_clients.push_back(cSock);
			printf("接收到新客户端<sock:%d>: IP = %s\n", (int)cSock, inet_ntoa(_cAddr.sin_addr));
		}

		//处理所有的客户端消息
		for (unsigned int n = 0; n < fdRead.fd_count; n++)
		{
			if (-1 == processor(fdRead.fd_array[n]))
			{
				//处理消息出现错误，在全局客户端数据里面删除它.
				auto iter = std::find(g_clients.begin(), g_clients.end(), fdRead.fd_array[n]);
				if (iter != g_clients.end())
				{
					g_clients.erase(iter);
				}
			}
		}
	}

	printf("任务结束.\n");
	//关闭所有的客户端连接
	for (int n = 0; n < g_clients.size(); n++)
	{
		closesocket(g_clients[n]);
	}
	//断开 socket 连接
	closesocket(_sock);


	WSACleanup();

	getchar();

	return 0;
}