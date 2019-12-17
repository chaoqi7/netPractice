#ifndef _CELL_SEND_BUFFER_H_
#define _CELL_SEND_BUFFER_H_

#include "CELLBuffer.hpp"

class CELLSendBuffer : public CELLBuffer
{
public:
	CELLSendBuffer(int nSize);
	~CELLSendBuffer();
	//写数据到指定 socket
	int Write2Socket(SOCKET sockfd);
	//写入一条消息
	int WriteData(netmsg_DataHeader* pHeader);
	//
	bool NeedWrite();
};

CELLSendBuffer::CELLSendBuffer(int nSize)
	:CELLBuffer(nSize)
{
}

CELLSendBuffer::~CELLSendBuffer()
{
}
inline int CELLSendBuffer::Write2Socket(SOCKET sockfd)
{
	int ret = 0;
	if (_nLast > 0 && sockfd != INVALID_SOCKET)
	{
		//发送数据
		ret = send(sockfd, _pBuf, _nLast, 0);
		//清空缓冲区
		_nLast = 0;
		if (SOCKET_ERROR == ret)
		{
			CELLLog::Info("sock=%d Write2Socket fail.\n", (int)sockfd);
		}
	}
	return ret;
}
inline int CELLSendBuffer::WriteData(netmsg_DataHeader * pHeader)
{
	return push((const char*)pHeader, pHeader->dataLength);
}
inline bool CELLSendBuffer::NeedWrite()
{
	return _nLast > 0;
}
#endif // _CELL_SEND_BUFFER_H_
