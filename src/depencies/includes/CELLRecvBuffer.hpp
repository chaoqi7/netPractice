#ifndef _CELL_RECV_BUFFER_H_
#define _CELL_RECV_BUFFER_H_

#include "CELLBuffer.hpp"

class CELLRecvBuffer : public CELLBuffer
{
public:
	CELLRecvBuffer(int nSize);
	~CELLRecvBuffer();
	//是否有一条消息
	bool hasMsg();
	//获取第一条消息
	netmsg_DataHeader* frontMsg();
	//删除第一条消息
	void popFrontMsg();
	//从指定 socket 读取数据
	int read4socket(SOCKET sockfd);
};

CELLRecvBuffer::CELLRecvBuffer(int nSize)
	:CELLBuffer(nSize)
{
}

CELLRecvBuffer::~CELLRecvBuffer()
{

}
inline bool CELLRecvBuffer::hasMsg()
{
	if (_nLast >= sizeof(netmsg_DataHeader))
	{
		netmsg_DataHeader* pHeader = (netmsg_DataHeader*)_pBuf;
		//是否有一条真正的消息长度
		return _nLast >= pHeader->dataLength;
	}
	return false;
}

inline netmsg_DataHeader * CELLRecvBuffer::frontMsg()
{
	return (netmsg_DataHeader*)_pBuf;
}
inline void CELLRecvBuffer::popFrontMsg()
{
	if (hasMsg())
	{
		pop(frontMsg()->dataLength);
	}
}
inline int CELLRecvBuffer::read4socket(SOCKET sockfd)
{
	int nLeft = _nCapacity - _nLast;
	if (nLeft > 0)
	{
		int nLen = (int)recv(sockfd, _pBuf + _nLast, nLeft, 0);
		if (nLen <= 0)
		{
			printf("<sock=%d> CELLRecvBuffer::read4socket error.\n", (int)sockfd);
			return nLen;
		}
		//扩大数据长度
		_nLast += nLen;
		return nLen;
	}
	return -1;
}
#endif // _CELL_RECV_BUFFER_H_
