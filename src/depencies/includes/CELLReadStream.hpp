#ifndef _CELL_READ_STREAM_H_
#define _CELL_READ_STREAM_H_

#include "CELLStream.hpp"
#include "NetMsg.h"

class CELLReadStream : public CELLStream
{
public:
	//方便直接使用 netmsg_DataHeader*
	CELLReadStream(netmsg_DataHeader* pHeader);
	//方便导出DLL的时候使用，
	CELLReadStream(char* pData, uint32_t nSize, bool bDelete = false);
	uint16_t ReadNetCMD();
	uint16_t ReadNetLength();
public:
	//bOffset 表示读取数据之后是否偏移，应对只读得情况。
	template<typename T>
	bool Read(T& nData = 0, bool bOffset = true)
	{
		uint32_t nLen = sizeof(T);
		//有数据可读
		if (canRead(nLen))
		{
			//读取数据
			memcpy(&nData, data() + _nReadPos, nLen);
			//偏移读取位置
			if (bOffset)
			{
				pop(nLen);
			}			
			return true;
		}
		return false;
	}
	/*
	nMaxNum 最大的元素个数
	*/
	template<typename T>
	uint32_t ReadArray(T* pData, uint32_t nMaxNum)
	{
		//读取元素个数
		uint32_t nElementNum = onlyRead();
		if (nElementNum <= nMaxNum)
		{
			//数据内容是否足够
			uint32_t nTotalLen = sizeof(T) * nElementNum;
			if (canRead(nTotalLen + sizeof(uint32_t)))
			{
				//首先偏移元素个数位置
				pop(sizeof(uint32_t));
				//读取数据
				memcpy(pData, data() + _nReadPos, nTotalLen);
				//偏移元素内容位置
				pop(nTotalLen);
				//返回读取的实际元素个数
				return nElementNum;
			}
		}
		return 0;
	}
public:
	//有符号基础类型
	int8_t ReadInt8();
	int16_t ReadInt16();
	int32_t ReadInt32();
	int64_t ReadInt64();
	//无符号基础类型
	uint8_t ReadUInt8();
	uint16_t ReadUInt16();
	uint32_t ReadUInt32();
	uint64_t ReadUInt64();

	float ReadFloat();
	double ReadDouble();

	//仅仅读取，不偏移
	uint32_t onlyRead();
private:
	//是否可以读取 nLen 字节
	bool canRead(uint32_t nLen);
	//读位置偏移
	void pop(uint32_t nLen);
private:
	int _nReadPos = 0;
};

inline CELLReadStream::CELLReadStream(netmsg_DataHeader * pHeader)
	:CELLReadStream((char*)pHeader, pHeader->dataLength, true)
{
}

inline CELLReadStream::CELLReadStream(char* pData, uint32_t nSize, bool bDelete)
	: CELLStream(pData, nSize, bDelete)
{
}

inline uint16_t CELLReadStream::ReadNetCMD()
{
	return ReadUInt16();
}

inline uint16_t CELLReadStream::ReadNetLength()
{
	return ReadUInt16();
}

inline int8_t CELLReadStream::ReadInt8()
{
	int8_t n = 0;
	Read(n);
	return n;
}

inline int16_t CELLReadStream::ReadInt16()
{
	int16_t n = 0;
	Read(n);
	return n;
}

inline int32_t CELLReadStream::ReadInt32()
{
	int32_t n = 0;
	Read(n);
	return n;
}

inline int64_t CELLReadStream::ReadInt64()
{
	int64_t n = 0;
	Read(n);
	return n;
}

inline uint8_t CELLReadStream::ReadUInt8()
{
	uint8_t n = 0;
	Read(n);
	return n;
}

inline uint16_t CELLReadStream::ReadUInt16()
{
	uint16_t n = 0;
	Read(n);
	return n;
}

inline uint32_t CELLReadStream::ReadUInt32()
{
	uint32_t n = 0;
	Read(n);
	return n;
}

inline uint64_t CELLReadStream::ReadUInt64()
{
	uint64_t n = 0;
	Read(n);
	return n;
}

inline float CELLReadStream::ReadFloat()
{
	float n = 0;
	Read(n);
	return n;
}

inline double CELLReadStream::ReadDouble()
{
	double n = 0;
	Read(n);
	return n;
}

inline uint32_t CELLReadStream::onlyRead()
{
	uint32_t n = 0;
	Read(n, false);
	return n;
}
inline bool CELLReadStream::canRead(uint32_t nLen)
{
	return _nReadPos + nLen <= capacity();
}
inline void CELLReadStream::pop(uint32_t nLen)
{
	_nReadPos += nLen;
}
#endif // _CELL_READ_STREAM_H_
