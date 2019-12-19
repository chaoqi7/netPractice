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
#endif // _CELL_BUFFER_H_
