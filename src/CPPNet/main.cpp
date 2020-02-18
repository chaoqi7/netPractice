#ifndef _CPP_NET_H_
#define _CPP_NET_H_

#include "CELLReadStream.hpp"
#include "EasySelectClient.hpp"

#ifdef _WIN32
#define EXPORT_DLL _declspec(dllexport)
#else
#define EXPORT_DLL
#endif

extern "C"
{
	typedef void(*OnNetMsgCallback)(void* csObj, void* pData, int nLen);
}

class NativeTCPClient : public EasySelectClient
{
public:
	void OnNetMsg(netmsg_DataHeader* pHeader) override
	{
		if (_cb)
		{
			_cb(_csObj, pHeader, pHeader->dataLength);
		}
	}

	void SetObject(void* csObj, OnNetMsgCallback cb)
	{
		_csObj = csObj;
		_cb = cb;
	}
private:
	void* _csObj;
	OnNetMsgCallback _cb;
};


extern "C"
{
	//////////////////////////////////////////////////////////////////////////
	//Native TCP Client.
	//create native TCP socket 
	EXPORT_DLL void* CELLNativeTCPClient_Create(void* csObj, OnNetMsgCallback cb, int sendSize, int recvSize)
	{
		NativeTCPClient* pClient = new NativeTCPClient();
		pClient->SetObject(csObj, cb);
		pClient->InitSocket(sendSize, recvSize);
		return pClient;
	}

	//use native socket connect server with ip, port.
	EXPORT_DLL bool CELLNativeTCPClient_Connect(NativeTCPClient* pClient, const char* ip, short port)
	{
		if (pClient)
		{
			return SOCKET_ERROR != pClient->Connect(ip, port);
		}
		return false;
	}
	//native socket work procedure.
	EXPORT_DLL bool CELLNativeTCPClient_OnRun(NativeTCPClient* pClient, int microseconds)
	{
		if (pClient)
		{
			pClient->OnRun(microseconds);
		}
		return false;
	}
	//close native TCP socket.
	EXPORT_DLL void CELLNativeTCPClient_Close(NativeTCPClient* pClient)
	{
		if (pClient)
		{
			pClient->Close();
			delete pClient;
		}
	}
	//use native TCP socket 2 send data.
	EXPORT_DLL int CELLNativeTCPClient_SendData(NativeTCPClient* pClient, void* pData, int nLen)
	{
		if (pClient)
		{
			return pClient->SendData((const char*)pData, nLen);
		}
		return 0;
	}

	EXPORT_DLL int CELLNativeTCPClient_SendWriteStream(NativeTCPClient* pClient, CELLWriteStream* writeStream)
	{
		if (pClient && writeStream)
		{
			return pClient->SendData(writeStream->Data(), writeStream->Length());
		}
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//CELLWriteStream.
	EXPORT_DLL void* CELLWriteStream_Create(int nSize)
	{
		CELLWriteStream* wStream = new CELLWriteStream(nSize);
		return wStream;
	}
	//有符号
	EXPORT_DLL bool CELLWriteStream_WriteInt8(CELLWriteStream* wStream, int8_t n)
	{
		if (wStream)
		{
			wStream->WriteInt8(n);
		}
		return false;
	}

	EXPORT_DLL bool CELLWriteStream_WriteInt16(CELLWriteStream* wStream, int16_t n)
	{
		if (wStream)
		{
			wStream->WriteInt16(n);
		}
		return false;
	}

	EXPORT_DLL bool CELLWriteStream_WriteInt32(CELLWriteStream* wStream, int32_t n)
	{
		if (wStream)
		{
			wStream->WriteInt32(n);
		}
		return false;
	}

	EXPORT_DLL bool CELLWriteStream_WriteInt64(CELLWriteStream* wStream, int64_t n)
	{
		if (wStream)
		{
			wStream->WriteInt64(n);
		}
		return false;
	}
	//无符号
	EXPORT_DLL bool CELLWriteStream_WriteUInt8(CELLWriteStream* wStream, uint8_t n)
	{
		if (wStream)
		{
			wStream->WriteUInt8(n);
		}
		return false;
	}

	EXPORT_DLL bool CELLWriteStream_WriteUInt16(CELLWriteStream* wStream, uint16_t n)
	{
		if (wStream)
		{
			wStream->WriteUInt16(n);
		}
		return false;
	}

	EXPORT_DLL bool CELLWriteStream_WriteUInt32(CELLWriteStream* wStream, uint32_t n)
	{
		if (wStream)
		{
			wStream->WriteUInt32(n);
		}
		return false;
	}

	EXPORT_DLL bool CELLWriteStream_WriteUInt64(CELLWriteStream* wStream, uint64_t n)
	{
		if (wStream)
		{
			wStream->WriteUInt64(n);
		}
		return false;
	}
	//单精度浮点
	EXPORT_DLL bool CELLWriteStream_WriteFloat(CELLWriteStream* wStream, float n)
	{
		if (wStream)
		{
			wStream->WriteFloat(n);
		}
		return false;
	}
	//双精度浮点
	EXPORT_DLL bool CELLWriteStream_WriteDouble(CELLWriteStream* wStream, double n)
	{
		if (wStream)
		{
			wStream->WriteDouble(n);
		}
		return false;
	}

	EXPORT_DLL bool CELLWriteStream_WriteString(CELLWriteStream* wStream, const char* str)
	{
		if (wStream && str)
		{
			wStream->WriteString(str);
		}
		return false;
	}

	EXPORT_DLL void CELLWriteStream_Release(CELLWriteStream* wStream)
	{
		if (wStream)
		{
			delete wStream;
		}
	}
	//////////////////////////////////////////////////////////////////////////
	//CELLReadStream.
	EXPORT_DLL void* CELLReadStream_Create(char* pData, int nLen)
	{
		CELLReadStream* rStream = new CELLReadStream(pData, nLen);
		return rStream;
	}

	EXPORT_DLL int8_t CELLReadStream_ReadInt8(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadInt8();
		}
		return 0;
	}

	EXPORT_DLL int16_t CELLReadStream_ReadInt16(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadInt16();
		}
		return 0;
	}

	EXPORT_DLL int32_t CELLReadStream_ReadInt32(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadInt32();
		}
		return 0;
	}

	EXPORT_DLL int64_t CELLReadStream_ReadInt64(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadInt64();
		}
		return 0;
	}

	EXPORT_DLL uint8_t CELLReadStream_ReadUInt8(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadUInt8();
		}
		return 0;
	}

	EXPORT_DLL uint16_t CELLReadStream_ReadUInt16(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadUInt16();
		}
		return 0;
	}

	EXPORT_DLL uint32_t CELLReadStream_ReadUInt32(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadUInt32();
		}
		return 0;
	}

	EXPORT_DLL uint64_t CELLReadStream_ReadUInt64(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadUInt64();
		}
		return 0;
	}

	EXPORT_DLL float CELLReadStream_ReadFloat(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadFloat();
		}
		return 0.0f;
	}

	EXPORT_DLL double CELLReadStream_ReadDouble(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->ReadDouble();
		}
		return 0.0;
	}

	EXPORT_DLL uint32_t CELLReadStream_ReadString(CELLReadStream* rStream, char* pBuff, int nLen)
	{
		if (rStream)
		{
			return rStream->ReadArray(pBuff, nLen);
		}
		return 0;
	}

	EXPORT_DLL uint32_t CELLReadStream_OnlyRead(CELLReadStream* rStream)
	{
		if (rStream)
		{
			return rStream->onlyRead();
		}
		return 0;
	}

	EXPORT_DLL void CELLReadStream_Release(CELLReadStream* rStream)
	{
		if (rStream)
		{
			delete rStream;
		}
	}
}

#endif //_CPP_NET_H_