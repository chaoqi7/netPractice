#ifndef _CELL_STREAM_H_
#define _CELL_STREAM_H_

#include <stdint.h>
#include <string.h>

class CELLStream
{
public:
	//供写调用
	CELLStream(uint32_t nSize = 1024);
	//供读调用
	CELLStream(char* pData, uint32_t nSize, bool bDelete = false);
	virtual ~CELLStream();
protected:
	char* data();
	uint32_t capacity();
private:
	//缓冲区数据
	char* _pBuf = nullptr;
	//最大容量
	uint32_t _nCapacity = 0;
	//是否删除
	bool _bDelete = true;
};

CELLStream::CELLStream(uint32_t nSize/* = 1024*/)
{
	_nCapacity = nSize;
	_pBuf = new char[_nCapacity];
	_bDelete = true;
}

inline CELLStream::CELLStream(char* pData, uint32_t nSize, bool bDelete/* = false*/)
{
	_nCapacity = nSize;
	_pBuf = pData;
	_bDelete = bDelete;
}

CELLStream::~CELLStream()
{
	if (_bDelete && _pBuf)
	{
		delete[] _pBuf;
		_pBuf = nullptr;
	}
}
inline char * CELLStream::data()
{
	return _pBuf;
}
inline uint32_t CELLStream::capacity()
{
	return _nCapacity;
}
#endif // _CELL_STREAM_H_
