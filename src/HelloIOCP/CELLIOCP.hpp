#ifndef _CELL_IOCP_HPP_
#define _CELL_IOCP_HPP_

#include <WinSock2.h>
#include "CELL.hpp"
#pragma comment(lib, "ws2_32.lib")

#define  IO_DATA_BUFFER_SIZE 1024

enum IO_TYPE
{
	ACCEPT = 10,
	RECV,
	SEND,
};

struct IO_DATA_BASE
{
	OVERLAPPED overlapped;
	SOCKET sockfd;
	char buf[IO_DATA_BUFFER_SIZE];
	int length;
	IO_TYPE iotype;
};

struct IOCP_EVENT
{
	//传入此指针的地址，返回被修改的值（指向实际的 overplapped）
	IO_DATA_BASE* pIoData;	
	SOCKET sockfd;
	DWORD bytesTrans;
};

class CELLIOCP
{
public:
	~CELLIOCP()
	{
		destory();
	}
	//创建 IOCP
	bool create()
	{		 
		_completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (_completionPort == NULL)
		{
			CELLLog_PError("CELLIOCP create fail.");
			return false;
		}
		return true;
	}

	void destory()
	{
		if (_completionPort)
		{
			CloseHandle(_completionPort);
			_completionPort = INVALID_HANDLE_VALUE;
		}		
	}

	//关联完成端口与 SOCKET
	bool reg(SOCKET sockfd)
	{	
		auto ret = CreateIoCompletionPort((HANDLE)sockfd, _completionPort, (ULONG_PTR)sockfd, 0);
		if (!ret)
		{
			CELLLog_PError("CELLIOCP reg fail.");
			return false;
		}
		return true;
	}

	bool loadAccept(SOCKET ListenSocket)
	{
		if (_serverSocket != INVALID_SOCKET)
		{
			CELLLog_Error("loadAccept _serverSocket != INVALID_SOCKET");
			return false;
		}
		if (_AcceptEx)
		{
			CELLLog_Error("loadAccept _AcceptEx != null");
			return false;
		}

		_serverSocket = ListenSocket;
		GUID GuidAcceptEx = WSAID_ACCEPTEX;
		DWORD dwBytes = 0;
		int iResult = WSAIoctl(ListenSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidAcceptEx, sizeof(GuidAcceptEx),
			&_AcceptEx, sizeof(_AcceptEx),
			&dwBytes, NULL, NULL);
		if (iResult == SOCKET_ERROR) {
			CELLLog_Error("WSAIoctl failed with error: %u\n", WSAGetLastError());
			return false;
		}

		return true;
	}

	void postAccept(IO_DATA_BASE* pIobase)
	{
		pIobase->sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		pIobase->iotype = IO_TYPE::ACCEPT;
		BOOL bRetVal = _AcceptEx(
			_serverSocket,
			pIobase->sockfd,
			pIobase->buf,
			0,	//0:不需要接收数据，有连接就直接通知；其它情况 sizeof(buf) - 2*(sizeof(sockaddr_in) + 16)
			sizeof(sockaddr_in) + 16,
			sizeof(sockaddr_in) + 16,
			NULL, //同步的时候返回实际接收到的数据，异步为空即可
			&pIobase->overlapped);

		if (FALSE == bRetVal)
		{
			auto errCode = WSAGetLastError();
			if (ERROR_IO_PENDING != errCode)
			{
				CELLLog_PError("postAccept AcceptEx.");
			}
		}
	}

	int wait(IOCP_EVENT& ioEvent, int timeout)
	{
		ioEvent.bytesTrans = 0;
		ioEvent.pIoData = nullptr;
		ioEvent.sockfd = INVALID_SOCKET;

		BOOL bRetVal = GetQueuedCompletionStatus(
			_completionPort,
			&ioEvent.bytesTrans,
			(PULONG_PTR)&ioEvent.sockfd,
			(LPOVERLAPPED*)&ioEvent.pIoData,
			timeout);
		if (FALSE == bRetVal)
		{
			auto errcode = GetLastError();
			if (WAIT_TIMEOUT == errcode)
			{
				return 0;
			}
			//客户端主动断开连接
			else if (ERROR_NETNAME_DELETED == errcode)
			{
				return 1;
			}
			CELLLog_PError("GetQueuedCompletionStatus error.");
			return -1;
		}

		return 1;
	}

	void postRecv(IO_DATA_BASE* pIobase)
	{
		pIobase->iotype = IO_TYPE::RECV;
		WSABUF wsBuf = {};
		wsBuf.buf = pIobase->buf;
		wsBuf.len = IO_DATA_BUFFER_SIZE;
		DWORD dwFlag = 0;
		int iRetVal = WSARecv(
			pIobase->sockfd,
			&wsBuf,
			1,
			NULL,
			&dwFlag,
			&pIobase->overlapped,
			NULL);

		if (SOCKET_ERROR == iRetVal)
		{
			auto errCode = WSAGetLastError();
			if (WSA_IO_PENDING != errCode)
			{
				CELLLog_PError("postRecv WSARecv");
			}
		}
	}

	void postSend(IO_DATA_BASE* pIobase)
	{
		pIobase->iotype = IO_TYPE::SEND;
		WSABUF wsBuf = {};
		wsBuf.buf = pIobase->buf;
		wsBuf.len = pIobase->length;
		DWORD dwFlag = 0;
		int iRetVal = WSASend(
			pIobase->sockfd,
			&wsBuf,
			1,
			NULL,
			dwFlag,
			&pIobase->overlapped,
			NULL);

		if (SOCKET_ERROR == iRetVal)
		{
			auto errCode = WSAGetLastError();
			if (WSA_IO_PENDING != errCode)
			{
				CELLLog_PError("postRecv WSASend");
			}
		}
	}

private:
	LPFN_ACCEPTEX _AcceptEx = NULL;
	SOCKET _serverSocket = INVALID_SOCKET;
	HANDLE _completionPort = INVALID_HANDLE_VALUE;
};

#endif // _CELL_IOCP_HPP_
