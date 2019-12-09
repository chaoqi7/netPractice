#ifndef _EASY_TCP_CLIENT_HPP_
#define _EASY_TCP_CLIENT_HPP_

#ifdef _WIN32
//windows
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
//linux and osx
#include <unistd.h>
#include <arpa/inet.h>
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define SOCKET int
#endif // _WIN32

#include <stdio.h>

#include "NetMsg.h"

class EasyTcpClient
{
public:
	EasyTcpClient();
	virtual ~EasyTcpClient();
	//连接远程服务器
	int Connect(const char* ip, unsigned short port);
	//关闭连接
	void Close();	
	//循环执行任务（当前使用select)
	bool OnRun();
	//是否运行
	bool IsRun();
	//发送消息
	int SendData(DataHeader* pHeader);
	int SendData(const char* pData, const int iLen);

private:
	//初始化 socket
	void InitSocket();
	//接收数据
	int RecvData();
	//处理消息
	virtual void OnNetMsg(DataHeader* pHeader);
private:
	SOCKET _sock;
	//消息缓冲区
	char _szMsgBuf[RECV_BUF_SIZE * 5] = {};
	//消息缓冲区消息的长度
	int _lastMsgPos = 0;
};

EasyTcpClient::EasyTcpClient()
{
	_sock = INVALID_SOCKET;
}

EasyTcpClient::~EasyTcpClient()
{
	Close();
}

inline void EasyTcpClient::InitSocket()
{
	if (IsRun())
	{
		printf("InitSocket Close old socket=%d.\n", (int)_sock);
		Close();
	}
#ifdef _WIN32
	WORD ver = MAKEWORD(2, 2);
	WSADATA dat = {};
	WSAStartup(ver, &dat);
#endif

	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == _sock)
	{
		printf("create socket fail.\n");
	}
	else {
		//printf("创建 socket=%d 成功.\n", (int)_sock);
	}
}

inline int EasyTcpClient::Connect(const char * ip, unsigned short port)
{
	if (!IsRun())
	{
		InitSocket();
	}

	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif
	int ret = connect(_sock, (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		printf("<sockt=%d>连接服务器<%s:%d>失败.\n", (int)_sock, ip, port);
		return -1;
	}
	else {
		//printf("<sockt=%d>连接服务器<%s:%d>成功.\n", (int)_sock, ip, port);
	}
	return ret;
}

inline void EasyTcpClient::Close()
{
	if (IsRun())
	{
#ifdef _WIN32
		closesocket(_sock);

		WSACleanup();
#else
		close(_sock);
#endif
		_sock = INVALID_SOCKET;
	}
}

inline bool EasyTcpClient::IsRun()
{
	return _sock != INVALID_SOCKET;
}

inline bool EasyTcpClient::OnRun()
{
	if (IsRun())
	{
		fd_set fdReads;
		FD_ZERO(&fdReads);
		FD_SET(_sock, &fdReads);
		timeval t = { 0, 0 };
		int ret = select((int)_sock + 1, &fdReads, nullptr, nullptr, &t);
		if (ret == SOCKET_ERROR)
		{
			printf("select error.\n");
			Close();
			return false;
		}
		else if (ret == 0) {
			//printf("select time out.\n");
			//continue;
		}

		if (FD_ISSET(_sock, &fdReads))
		{
			FD_CLR(_sock, &fdReads);
			if (-1 == RecvData())
			{
				printf("<sockt=%d>与服务器断开连接.\n", (int)_sock);
				Close();
				return false;
			}
		}
		return true;
	}
	return false;
}

inline int EasyTcpClient::SendData(DataHeader * pHeader)
{
	if (IsRun() && pHeader)
	{
		return send(_sock, (const char*)pHeader, pHeader->dataLength, 0);
	}
	return SOCKET_ERROR;
}

inline int EasyTcpClient::SendData(const char * pData, const int iLen)
{
	if (IsRun() && pData)
	{
		return send(_sock, pData, iLen, 0);
	}
	return SOCKET_ERROR;
}

inline int EasyTcpClient::RecvData()
{
	//一次性从 socket 缓冲区里面读取最大的数据
	char* szRecv = _szMsgBuf + _lastMsgPos;
	int nLen = recv(_sock, szRecv, RECV_BUF_SIZE * 5 - _lastMsgPos, 0);
	if (nLen <= 0)
	{
		printf("<sock=%d>与服务器断开连接，任务结束.\n", (int)_sock);
		return -1;
	}
	//当前未处理的消息长度 + nLen
	_lastMsgPos += nLen;
	//是否有一个消息头长度
	while (_lastMsgPos >= sizeof(DataHeader))
	{
		DataHeader* pHeader = (DataHeader*)_szMsgBuf;
		//是否有一条真正的消息长度
		if (_lastMsgPos >= pHeader->dataLength)
		{
			//剩余未处理的消息长度
			unsigned int nLeftSize = _lastMsgPos - pHeader->dataLength;
			//处理消息
			OnNetMsg(pHeader);
			if (nLeftSize > 0)
			{
				//未处理的消息前移
				memcpy(_szMsgBuf, _szMsgBuf + pHeader->dataLength, nLeftSize);
				//更新未处理消息长度
				_lastMsgPos = nLeftSize;
			}
			else {
				_lastMsgPos = 0;
			}
			
		}
		else {
			//不够一条消息
			break;
		}
	}

	return 0;
}

inline void EasyTcpClient::OnNetMsg(DataHeader* pHeader)
{
	switch (pHeader->cmd)
	{
	case CMD_LOGIN_RESULT:
	{		
		LoginResult* pLoginResult = (LoginResult*)pHeader;
		//printf("<sockt=%d>收到服务器返回消息 CMD_LOGIN_RESULT, Result:%d, len:%d\n",
		//	(int)_sock, pLoginResult->result, pLoginResult->dataLength);
	}
	break;
	case CMD_LOGINOUT_RESULT:
	{
		LogoutResult* pLogoutResult = (LogoutResult*)pHeader;
		//printf("<sockt=%d>收到服务器返回消息 CMD_LOGINOUT_RESULT, Result:%d, len:%d\n",
		//	(int)_sock, pLogoutResult->result, pLogoutResult->dataLength);
	}
	break;
	case CMD_NEW_USER_JOIN:
	{
		NewUserJoin* pUserJoin = (NewUserJoin*)pHeader;
		//printf("<sockt=%d>收到服务器返回消息 CMD_NEW_USER_JOIN, sock:%d, len:%d\n",
		//	(int)_sock, pUserJoin->sock, pUserJoin->dataLength);
	}
	break;
	case CMD_ERROR:
	{
		printf("CMD_ERROR...\n");
	}
	break;
	default:
		printf("收到未定义消息.\n");
		break;
	}
}

#endif //_EASY_TCP_CLIENT_HPP_