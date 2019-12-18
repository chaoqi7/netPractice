#ifndef _CELL_BUFFER_H_
#define _CELL_BUFFER_H_

#include "CELL.hpp"

class CELLBuffer
{
public:
	CELLBuffer(int nSize);
	virtual ~CELLBuffer();
	//添加数据
	int push(const char* pData, int nLen);
	//删除头部指定长度数据
	void pop(int nLen);
protected:
	//缓冲区数据
	char* _pBuf = nullptr;
	//消息尾部位置
	int _nLast = 0;
	//最大容量
	int _nCapacity = 0;
};

CELLBuffer::CELLBuffer(int nSize)
{
	_nCapacity = nSize;
	_nLast = 0;
	_pBuf = new char[_nCapacity];
}

CELLBuffer::~CELLBuffer()
{
	if (_pBuf)
	{
		delete[] _pBuf;
		_pBuf = nullptr;
	}
}

inline int CELLBuffer::push(const char * pData, int nLen)
{
	//是否需要拓展空间
	if (_nLast + nLen > _nCapacity)
	{
		int n = _nCapacity - _nLast;
		if (n < 8192)
		{
			n = 8192;
		}
		char* pNewBuf = new char[_nCapacity + n];
		memcpy(pNewBuf, _pBuf, _nLast);
		delete[] _pBuf;
		_pBuf = pNewBuf;
	}

	if ((_nLast + nLen) <= _nCapacity)
	{
		memcpy(_pBuf + _nLast, pData, nLen);
		_nLast += nLen;
		return nLen;
	}
	else {
		CELLLog::Info("###CELLBUFFER push.ERROR FULL.nLast=%d, nCapacity=%d, nLen=%d\n",
			_nLast, _nCapacity, nLen);
	}

	return -1;
}

inline void CELLBuffer::pop(int nLen)
{
	int n = _nLast - nLen;
	if (n > 0)
	{
		memcpy(_pBuf, _pBuf + nLen, n);
	}

	_nLast = n;
}

//////////////////////////////////////////////////////////////////////////
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
			CELLLog::Info("<sock=%d> CELLRecvBuffer::read4socket error.\n", (int)sockfd);
			return nLen;
		}
		//扩大数据长度
		_nLast += nLen;
		return nLen;
	}
	return -1;
}

//////////////////////////////////////////////////////////////////////////
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

#endif // _CELL_BUFFER_H_
