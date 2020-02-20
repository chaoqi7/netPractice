
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#include "CELL.hpp"
#include "CELLLog.hpp"
#include "CELLNetWork.hpp"

#include "CELLIOCP.hpp"

int main()
{
	CELLLog::setLogPath("helloIocp", "w", false);
	CELLNetWork::Init();
	//建立socket
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == listenSocket)
	{
		CELLLog_PError("绑定端口失败.");
		return -1;
	}
	else
	{
		CELLLog_Info("绑定 socket=%d 成功.", (int)listenSocket);
	}

	sockaddr_in _sAddr = {};
	_sAddr.sin_family = AF_INET;
	_sAddr.sin_port = htons(4567);
	_sAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	int ret = bind(listenSocket, (sockaddr *)&_sAddr, sizeof(_sAddr));
	if (SOCKET_ERROR == ret)
	{
		CELLLog_PError("绑定端口错误.");
		return -1;
	}
	else
	{
		CELLLog_Info("绑定端口成功.");
	}

	if (listenSocket != INVALID_SOCKET)
	{
		ret = listen(listenSocket, 5);
		if (SOCKET_ERROR == ret)
		{
			CELLLog_PError("监听网络<socket=%d>端口错误.", (int)listenSocket);
			return -1;
		}
		else
		{
			CELLLog_Info("监听网络<socket=%d>端口成功.", (int)listenSocket);
		}
	}

	//创建 IOCP 
	CELLIOCP iocp;
	iocp.create();
	//关联完成端口与 SERVERSOCKET
	iocp.reg(listenSocket);
	iocp.loadAccept(listenSocket);
	
	IO_DATA_BASE iobase = {};
	iocp.postAccept(&iobase);
		
	IOCP_EVENT events = {};

	while (true)
	{
		int ret = iocp.wait(events, 1);
		if (ret < 0)
		{
			break;
		}
		else if (ret == 0)
		{
			continue;
		}

		if (IO_TYPE::ACCEPT == events.pIoData->iotype)
		{
			CELLLog_Info("ACCEPT...sockfd=%d, bytesTrans=%d", 
				events.pIoData->sockfd, events.bytesTrans);
			//关联完成端口与 ClientSocket
			iocp.reg(events.pIoData->sockfd);

			iocp.postRecv(events.pIoData);
		}
		else if (IO_TYPE::RECV == events.pIoData->iotype)
		{
			if (events.bytesTrans <= 0)
			{
				CELLLog_PError("RECV COMPLETION sockfd=%d, bytesTrans=%d", 
					events.pIoData->sockfd, events.bytesTrans);
				CELLNetWork::destorySocket(events.pIoData->sockfd);
				iocp.postAccept(events.pIoData);
				continue;
			}
			CELLLog_Info("hello iocp RECV...sockfd=%d, bytesTrans=%d", events.pIoData->sockfd, events.bytesTrans);
			events.pIoData->length = events.bytesTrans;
			iocp.postSend(events.pIoData);
		}
		else if (IO_TYPE::SEND == events.pIoData->iotype)
		{
			if (events.bytesTrans <= 0)
			{
				CELLLog_PError("SEND COMPLETION sockfd=%d, bytesTrans=%d", 
					events.pIoData->sockfd, events.bytesTrans);
				CELLNetWork::destorySocket(events.pIoData->sockfd);
				continue;
			}
			CELLLog_Info("SEND...sockfd=%d, bytesTrans=%d", events.pIoData->sockfd, events.bytesTrans);
			iocp.postRecv(events.pIoData);
		}
		else {
			CELLLog_Error("Undefine ....");
		}
	}

	//close the server socket.
	CELLNetWork::destorySocket(listenSocket);
	//close the completion port handle.
	iocp.destory();

	getchar();
	return 0;
}
