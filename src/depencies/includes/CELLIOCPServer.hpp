#ifndef _CELL_IOCP_SERVER_HPP_
#define _CELL_IOCP_SERVER_HPP_

#include <algorithm>

#include "CELLIOCP.hpp"
#include "CELLServer.hpp"

class CELLIOCPServer : public CellServer
{
public:
	CELLIOCPServer()
	{
		_iocp.create();
	}

	~CELLIOCPServer()
	{
		Close();
	}

	void OnClientJoin(CELLClient *pClient) override
	{		
		//
		_iocp.reg(pClient->socketfd(), pClient);		
		//关注数据接收事件	
		auto pIoData = pClient->makeRecvData();
		if (pIoData)
		{
			_iocp.postRecv(pIoData);
		}		
		//
		CellServer::OnClientJoin(pClient);
	}

	void rmClient(CELLClient *pClient)
	{
		auto iter = std::find(_clients.begin(), _clients.end(), pClient);
		if (iter != _clients.end())
		{
			_clients.erase(iter);
		}
		OnClientLeave(pClient);
	}

	//通过select 检测可写，可读。
	bool DoNetEvents()
	{
		//检查需要可写数据到客户端
		for (size_t n = 0; n < _clients.size(); n++)
		{
			if (_clients[n]->NeedWrite())
			{
				auto pSendIoData = _clients[n]->makeSendData();
				if (pSendIoData)
				{
					if (!_iocp.postSend(pSendIoData))
						rmClient(_clients[n]);
				}
			}
			
			auto pRecvIoData = _clients[n]->makeRecvData();
			if (pRecvIoData)
			{
				if (_iocp.postRecv(pRecvIoData))
					rmClient(_clients[n]);
			}
		}

		while (true)
		{
			int ret = doIocpNetEvents();
			if (ret < 0)
			{
				return false;
			}
			else if (ret == 0)
			{
				break;
			}
			else {
				//have action to do.
			}
		}


		return true;
	}

	int doIocpNetEvents()
	{
		int ret = _iocp.wait(_ioEvent, 1);
		if (ret < 0)
		{
			CELLLog_PError("CELLIOCPServer::doIocpEvents wait.");
			return -1;
		}
		else if (0 == ret)
		{
			return 0;
		}

		if (IO_TYPE::RECV == _ioEvent.pIoData->iotype)
		{
			CELLClient* pClient = (CELLClient*)_ioEvent.data.ptr;
			if (_ioEvent.bytesTrans <= 0)
			{
				CELLLog_PError("doIocpNetEvents RECV sockfd=%d, bytesTrans=%d",
					_ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);
				rmClient(pClient);
				return -1;
			}
			//CELLLog_Info("doIocpNetEvents RECV...sockfd=%d, bytesTrans=%d", _ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);			
			if (pClient)
			{
				OnNetRecv(pClient);
				pClient->read4iocp(_ioEvent.bytesTrans);
			}
		}
 		else if (IO_TYPE::SEND == _ioEvent.pIoData->iotype)
 		{
 			CELLClient* pClient = (CELLClient*)_ioEvent.data.ptr;
 			if (_ioEvent.bytesTrans <= 0)
 			{
 				CELLLog_PError("doIocpNetEvents SEND sockfd=%d, bytesTrans=%d",
 					_ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);
 				rmClient(pClient);
 				return -1;
 			}
 			//CELLLog_Info("doIocpNetEvents SEND...sockfd=%d, bytesTrans=%d", _ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);
 			if (pClient)
 			{
 				pClient->send2iocp(_ioEvent.bytesTrans);
 			}
 		}
		else {
			CELLLog_Error("doIocpNetEvents unknown action.");
		}

		return 1;
	}

private:
	CELLIOCP _iocp;
	IOCP_EVENT _ioEvent = {};
};

#endif // _CELL_IOCP_SERVER_HPP_
