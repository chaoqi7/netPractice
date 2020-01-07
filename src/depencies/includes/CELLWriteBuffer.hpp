#ifndef _CELL_WRITE_BUFFER_H_
#define _CELL_WRITE_BUFFER_H_

#include "CELLBuffer.hpp"

class CELLWriteBuffer : public CELLBuffer
{
public:
	CELLWriteBuffer(int nSize);
	~CELLWriteBuffer();
	//写数据到指定 socket
	int Write2Socket(SOCKET sockfd);
	//写入一条消息
	int WriteData(netmsg_DataHeader* pHeader);
	int WriteData(const char* pData, int nLen);
	//
	bool NeedWrite();
};

CELLWriteBuffer::CELLWriteBuffer(int nSize)
	:CELLBuffer(nSize)
{
}

CELLWriteBuffer::~CELLWriteBuffer()
{
}
inline int CELLWriteBuffer::Write2Socket(SOCKET sockfd)
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
			CELLLog_Error("sock=%d CELLWriteBuffer Write2Socket fail.", (int)sockfd);
		}
	}
	return ret;
}
inline int CELLWriteBuffer::WriteData(netmsg_DataHeader * pHeader)
{
	return push((const char*)pHeader, pHeader->dataLength);
}
inline int CELLWriteBuffer::WriteData(const char * pData, int nLen)
{
	return push(pData, nLen);
}
inline bool CELLWriteBuffer::NeedWrite()
{
	return _nLast > 0;
}

#endif // _CELL_BUFFER_H_
