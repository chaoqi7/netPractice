#ifndef _CELL_SELECT_SERVER_HPP_
#define _CELL_SELECT_SERVER_HPP_

#include "CELLFDSet.hpp"
#include "CELLServer.hpp"

class CellSelectServer : public CellServer
{
public:
	~CellSelectServer()
	{
		Close();
	}
	//通过select 检测可写，可读。
	bool DoNetEvents();
	//当前 socket 触发了可读
	void HandleReadEvent();
	//当前 socket 触发了可写
	void HandleWriteEvent();
private:
	CELLFDSet _fdRead;
	CELLFDSet _fdWrite;
	//查询的客户端集合的备份
	CELLFDSet _fdReadBack;
	//当前最大的 socket
	SOCKET _maxSocket;
};

inline bool CellSelectServer::DoNetEvents()
{
	if (_clientChange)
	{
		_clientChange = false;
		_fdRead.zero();
		//nfds 当前 socket 最大值+1（兼容贝克利套接字）. 在 windows 里面可以设置为 0.
		_maxSocket = _clients[0]->getSocketfd();
		//把全局客户端数据加入可读监听部分
		for (size_t n = 0; n < _clients.size(); n++)
		{
			_fdRead.add(_clients[n]->getSocketfd());
			if (_maxSocket < _clients[n]->getSocketfd())
			{
				_maxSocket = _clients[n]->getSocketfd();
			}
		}
		_fdReadBack.copyfrom(_fdRead);
	}
	else {
		_fdRead.copyfrom(_fdReadBack);
	}

	//检查需要可写数据到客户端
	bool bNeedWrite = false;
	_fdWrite.zero();
	for (size_t n = 0; n < _clients.size(); n++)
	{
		if (_clients[n]->NeedWrite())
		{
			bNeedWrite = true;
			_fdWrite.add(_clients[n]->getSocketfd());
		}
	}
	/*
	NULL:一直阻塞
	timeval 只能精确到秒
	*/
	timeval t = { 0, 1 };
	int ret = 0;
	if (bNeedWrite)
	{
		ret = select((int)_maxSocket + 1, _fdRead.fdset(), _fdWrite.fdset(), nullptr, &t);
	}
	else {
		ret = select((int)_maxSocket + 1, _fdRead.fdset(), nullptr, nullptr, &t);
	}

	if (SOCKET_ERROR == ret)
	{
		CELLLog_PError("CellSelectServer::DoNetEvents select.");
		return false;
	}
	else if (0 == ret) {
		return true;
	}

	//处理可读
	HandleReadEvent();
	//处理可写
	HandleWriteEvent();

	return true;
}

inline void CellSelectServer::HandleReadEvent()
{
	for (size_t n = 0; n < _clients.size(); n++)
	{
		SOCKET curfd = _clients[n]->getSocketfd();
		if (_fdRead.has(curfd))
		{
			//FD_CLR(curfd, &fdRead);
			if (SOCKET_ERROR == RecvData(_clients[n]))
			{
				//处理消息出现错误，在全局客户端数据里面删除它.
				auto iter = _clients.begin() + n;
				if (iter != _clients.end())
				{
					OnClientLeave(_clients[n]);
					_clients.erase(iter);
				}
			}
		}
	}
}

inline void CellSelectServer::HandleWriteEvent()
{
	for (size_t n = 0; n < _clients.size(); n++)
	{
		SOCKET curfd = _clients[n]->getSocketfd();
		if (_clients[n]->NeedWrite() && _fdWrite.has(curfd))
		{
			//FD_CLR(curfd, &fdWrite);
			if (SOCKET_ERROR == _clients[n]->SendDataReal())
			{
				auto iter = _clients.begin() + n;
				if (iter != _clients.end())
				{
					OnClientLeave(_clients[n]);
					_clients.erase(iter);
				}
			}
		}
	}
}

#endif // _CELL_SELECT_SERVER_HPP_
