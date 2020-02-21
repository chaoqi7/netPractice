#ifndef _CELL_READ_BUFFER_H_
#define _CELL_READ_BUFFER_H_

#include "CELLBuffer.hpp"

class CELLReadBuffer : public CELLBuffer
{
public:
	CELLReadBuffer(int nSize);
	//是否有一条消息
	bool hasMsg();
	//获取第一条消息
	netmsg_DataHeader* frontMsg();
	//删除第一条消息
	void popFrontMsg();
	//从指定 socket 读取数据
	int read4socket(SOCKET sockfd);

#ifdef _USE_IOCP_
	IO_DATA_BASE* makeRecvData(SOCKET sockfd)
	{
		int nLeft = _nCapacity - _nLast;
		if (nLeft > 0)
		{
			_ioData.wsabuf.buf = _pBuf + _nLast;
			_ioData.wsabuf.len = nLeft;
			_ioData.sockfd = sockfd;
			return &_ioData;
		}
		return nullptr;
	}

	bool read4iocp(int nRead)
	{
		int nLeft = _nCapacity - _nLast;

		if (nRead > 0 && nLeft > 0)
		{			
			if (nLeft >= nRead)
			{
				_nLast += nRead;
				return true;
			}			
		}
		CELLLog_Error("CELLReadBuffer read4iocp error sock=%d, nLeft=%d, nRead=%d.", _ioData.sockfd, nLeft, nRead);
		return false;
	}
private:
	IO_DATA_BASE _ioData = {};
#endif	
};

CELLReadBuffer::CELLReadBuffer(int nSize)
	:CELLBuffer(nSize)
{
}

inline bool CELLReadBuffer::hasMsg()
{
	if (_nLast >= sizeof(netmsg_DataHeader))
	{
		netmsg_DataHeader* pHeader = (netmsg_DataHeader*)_pBuf;
		//是否有一条真正的消息长度
		return _nLast >= pHeader->dataLength;
	}
	return false;
}

inline netmsg_DataHeader * CELLReadBuffer::frontMsg()
{
	return (netmsg_DataHeader*)_pBuf;
}
inline void CELLReadBuffer::popFrontMsg()
{
	if (hasMsg())
	{
		pop(frontMsg()->dataLength);
	}
}
inline int CELLReadBuffer::read4socket(SOCKET sockfd)
{
	int nLeft = _nCapacity - _nLast;
	if (nLeft > 0)
	{
		int nLen = (int)recv(sockfd, _pBuf + _nLast, nLeft, 0);
		if (nLen <= 0)
		{
			//CELLLog_PError("<sock=%d> CELLReadBuffer::read4socket.", (int)sockfd);
			return nLen;
		}
		//扩大数据长度
		_nLast += nLen;
		return nLen;
	}
	return 0;
}
#endif // _CELL_READ_BUFFER_H_
