#ifndef _EASY_SELECT_CLIENT_HPP_
#define _EASY_SELECT_CLIENT_HPP_

#include "EasyTcpClient.hpp"
#include "CELLFDSet.hpp"

class EasySelectClient : public EasyTcpClient
{
public:
	//循环执行任务（当前使用select)
	bool OnRun(int microseconds) override
	{
		if (!IsRun())
		{
			return false;
		}

		SOCKET cSock = _pClient->socketfd();
		_fdRead.zero();
		_fdRead.add(cSock);

		_fdWrite.zero();

		timeval t = { 0, microseconds };
		int ret = 0;
		if (_pClient->NeedWrite())
		{
			_fdWrite.add(cSock);
			ret = select((int)cSock + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &t);
		}
		else
		{
			ret = select((int)cSock + 1, _fdRead.fdset(), nullptr, nullptr, &t);
		}

		if (ret == SOCKET_ERROR)
		{
			CELLLog_PError("EasySelectClient::OnRun select.");
			Close();
			return false;
		}
		else if (ret == 0)
		{
			//CELLLog_Info("select time out.");
			//continue;
		}

		if (_fdRead.has(cSock))
		{
			if (-1 == RecvData())
			{
				//CELLLog_PError("<sockt=%d> EasyTcpClient::OnRun RecvData.", (int)cSock);
				Close();
				return false;
			}
		}

		if (_fdWrite.has(cSock))
		{
			if (-1 == _pClient->SendDataReal())
			{
				CELLLog_PError("<sockt=%d> EasyTcpClient::OnRun SendDataReal.", (int)cSock);
				Close();
				return false;
			}
		}
		return true;
	}

private:
	CELLFDSet _fdRead;
	CELLFDSet _fdWrite;
};

#endif //_EASY_SELECT_CLIENT_HPP_