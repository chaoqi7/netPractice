#ifndef _EASY_IOCP_SERVER_HPP_
#define _EASY_IOCP_SERVER_HPP_

//IOCP 专用宏
#define _USE_IOCP_

#include "EasyTcpServer.hpp"
#include "CELLIOCP.hpp"
#include "CELLIOCPServer.hpp"
//#include "CELLSelectServer.hpp"

class EasyIOCPServer : public EasyTcpServer
{
public:
	~EasyIOCPServer()
	{
		Close();
	}
	void Start(int cellServerCount = 1)
	{
		EasyTcpServer::Start<CELLIOCPServer>(cellServerCount);
	}

protected:
	//工作函数
	void OnRun(CELLThread *pThread)
	{
		CELLIOCP iocp;
		iocp.create();
		iocp.reg(socketfd());
		//关注新客户端接入事件
		iocp.loadAccept(socketfd());

		IO_DATA_BASE iobase = {};	
		const int len = 1024;// -2 * (sizeof(sockaddr_in) + 16);
		char buf[len] = {};
		iobase.wsabuf.buf = buf;
		iobase.wsabuf.len = len;
		iocp.postAccept(&iobase);

		IOCP_EVENT ioEvent = {};

		while (pThread->IsRun())
		{
			time4msg();

			int ret = iocp.wait(ioEvent, 1);
			if (ret < 0)
			{
				CELLLog_PError("EasyEpollServer::OnRun wait.");
				pThread->Exit();
				break;
			}
			else if (ret == 0) {
				continue;
			}
			
			if (IO_TYPE::ACCEPT == ioEvent.pIoData->iotype)
			{
				//CELLLog_Info("new client join sock=%d", ioEvent.pIoData->sockfd);
				//
				IocpAccept(ioEvent.pIoData->sockfd);
				//继续投递接收新客户端连接
				iocp.postAccept(&iobase);
			}
		}
	}


	int IocpAccept(SOCKET cSock)
	{
		if (INVALID_SOCKET == cSock)
		{
			CELLLog_PError("接受到无效客户端SOCKET");
			return -1;
		}
		else
		{
			if (_clientCount < _nMaxClient)
			{
				//把新客户端 socket 添加到全局数据里面
				AddClient2CellServer(new CELLClient(cSock, _nSendBufSize, _nRecvBufSize));
			}
			else
			{
				CELLNetWork::destorySocket(cSock);
				CELLLog_Error("Accept to nMaxClient");
			}
		}

		return 0;
	}
};


#endif // !_EASY_EPOLL_SERVER_HPP_
