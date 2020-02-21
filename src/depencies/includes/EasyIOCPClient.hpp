#ifndef _EASY_IOCP_CLIENT_HPP_
#define _EASY_IOCP_CLIENT_HPP_

#ifndef _USE_IOCP_
#define _USE_IOCP_
#endif 

#include "EasyTcpClient.hpp"
#include "CELLIOCP.hpp"

class EasyIOCPClient : public EasyTcpClient
{
public:
	void OnInitSocketComplete() override
	{
		_iocp.create();
		_iocp.reg(_pClient->socketfd(), _pClient);
	}

	bool postWrite()
	{
		if (!_pClient->isPostSend())
		{
			auto pIoData = _pClient->makeSendData();
			if (pIoData)
			{
				if (!_iocp.postSend(pIoData))
				{
					Close();
					return false;
				}
			}
		}
		return true;
	}

	bool postRead()
	{
		if (!_pClient->isPostRead())
		{
			auto pIoData = _pClient->makeRecvData();
			if (pIoData)
			{
				if (!_iocp.postRecv(pIoData))
				{
					Close();
					return false;
				}
			}
		}
		return true;
	}
	//循环执行任务（当前使用select)
	bool OnRun(int microseconds)
	{
		if (!IsRun())
		{
			return false;
		}

		if (_pClient->NeedWrite())
		{
			if (!postWrite())
			{
				return false;
			}

			if (!postRead())
			{
				return false;
			}
		}
		else
		{
			if (!postRead())
			{
				return false;
			}
		}

		while (true)
		{
			int ret = doIocpNetEvents(microseconds);
			if (ret < 0)
			{
				return false;
			}
			else if (ret == 0)
			{
				DoMsg();
				break;
			}
			else {
				//have action to do.
			}
		}

		return true;
	}

	int doIocpNetEvents(int microseconds)
	{
		int ret = _iocp.wait(_ioEvent, microseconds);
		if (ret < 0)
		{
			CELLLog_PError("CELLIOCPClient doIocpEvents wait.");
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
				CELLLog_Error("CELLIOCPClient doIocpNetEvents RECV sockfd=%d, bytesTrans=%d",
					_ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);
				Close();
				return -1;
			}
			//CELLLog_Info("CELLIOCPClient doIocpNetEvents RECV...sockfd=%d, bytesTrans=%d", _ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);			
			if (pClient)
			{
				pClient->read4iocp(_ioEvent.bytesTrans);
			}
		}
		else if (IO_TYPE::SEND == _ioEvent.pIoData->iotype)
		{
			CELLClient* pClient = (CELLClient*)_ioEvent.data.ptr;
			if (_ioEvent.bytesTrans <= 0)
			{
				CELLLog_Error("CELLIOCPClient doIocpNetEvents SEND sockfd=%d, bytesTrans=%d",
					_ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);
				Close();
				return -1;
			}
			//CELLLog_Info("CELLIOCPClient doIocpNetEvents SEND...sockfd=%d, bytesTrans=%d", _ioEvent.pIoData->sockfd, _ioEvent.bytesTrans);
			if (pClient)
			{
				pClient->send2iocp(_ioEvent.bytesTrans);
			}
		}
		else {
			CELLLog_Error("CELLIOCPClient doIocpNetEvents unknown io type.");
			return -1;
		}

		return 1;
	}

private:
	CELLIOCP _iocp;
	IOCP_EVENT _ioEvent = {};
};


#endif //_EASY_IOCP_CLIENT_HPP_