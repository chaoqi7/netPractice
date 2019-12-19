#ifndef _CELL_WRITE_STREAM_H_
#define _CELL_WRITE_STREAM_H_

#include "CELLStream.hpp"

class CELLWriteStream : public CELLStream
{
public:
	CELLWriteStream(int nSize = 1024);	
	//写入消息头ID
	bool WriteNetCMD(uint16_t cmd);
	//获取消息指针
	char* Data();
	//获取消息长度
	uint16_t Length();
	//写入消息最终长度
	void Finish();
public:
	//有符号基础类型
	bool WriteInt8(int8_t n);
	bool WriteInt16(int16_t n);
	bool WriteInt32(int32_t n);
	bool WriteInt64(int64_t n);
	//无符号基础类型
	bool WriteUInt8(uint8_t n);
	bool WriteUInt16(uint16_t n);
	bool WriteUInt32(uint32_t n);
	bool WriteUInt64(uint64_t n);
	//浮点数
	bool WriteFloat(float n);
	bool WriteDouble(double n);
	//字符串
	bool WriteString(const char* pData);
public:
	template<typename T>
	bool Write(T n)
	{
		//判断是否有空间可写
		uint32_t nLen = sizeof(T);
		if (canWrite(nLen))
		{
			//写入数据
			memcpy(data() + _nWritePos, &n, nLen);
			push(nLen);
			return true;
		}
		return false;
	}
	template<typename T>
	bool WriteArray(T* pData, uint32_t nEelmentNum)
	{
		//判断是否有空间可写
		uint32_t nTotalLen = sizeof(T) * nEelmentNum;
		if (canWrite(nTotalLen + sizeof(uint32_t)))
		{
			//写入数组元素个数
			Write<uint32_t>(nEelmentNum);
			//写入数组具体内容
			memcpy(data() + _nWritePos, pData, nTotalLen);
			push(nTotalLen);
			return true;
		}
		return false;
	}
private:
	void push(int nLen);
	bool canWrite(int nLen);
	uint32_t getWritePos();
	void setWritePos(uint32_t);
private:
	uint32_t _nWritePos = 0;
};

inline bool CELLWriteStream::WriteNetCMD(uint16_t cmd)
{
	return Write(cmd);
}

inline char * CELLWriteStream::Data()
{
	return data();
}

inline uint16_t CELLWriteStream::Length()
{
	return _nWritePos;
}

inline void CELLWriteStream::Finish()
{
	uint32_t pos = getWritePos();
	setWritePos(0);
	Write<uint16_t>(pos);
	setWritePos(pos);
}

inline bool CELLWriteStream::WriteInt8(int8_t n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteInt16(int16_t n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteInt32(int32_t n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteInt64(int64_t n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteUInt8(uint8_t n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteUInt16(uint16_t n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteUInt32(uint32_t n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteUInt64(uint64_t n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteFloat(float n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteDouble(double n)
{
	return Write(n);
}

inline bool CELLWriteStream::WriteString(const char * pData)
{
	return WriteArray(pData, (uint32_t)strlen(pData));
}

CELLWriteStream::CELLWriteStream(int nSize/* = 1024*/)
	:CELLStream(nSize)
{
	//需要先写入一个消息长度位置
	Write<uint16_t>(0);
}

inline void CELLWriteStream::push(int nLen)
{
	_nWritePos += nLen;
}

inline bool CELLWriteStream::canWrite(int nLen)
{
	return _nWritePos + nLen <= capacity();
}

inline uint32_t CELLWriteStream::getWritePos()
{
	return _nWritePos;
}

inline void CELLWriteStream::setWritePos(uint32_t newPos)
{
	_nWritePos = newPos;
}

#endif // _CELL_WRITE_STREAM_H_