#ifndef _CELL_CLIENT_HPP_
#define _CELL_CLIENT_HPP_
//心跳检测死亡倒计时时间
#define CLIENT_HEART_DEATH_TIME 60 * 1000
//定时把发送缓冲区数据发送到客户端时间(200毫秒是最小值)
#define FLUSH_SEND_BUF_2_CLIENT_TIME 200

#include "CELL.hpp"

class CELLClient
{
public:
	CELLClient(SOCKET cSock);
	~CELLClient();
	//获取当前客户端的 socket
	SOCKET GetSocketfd();
	//获取当前消息的最后索引
	int GetLastRecvPos();
	//设置当前消息的最后索引
	void SetLastRecvPos(int newPos);
	//获取当前消息缓冲区
	char* RecvBuf();
	//发送消息
	int SendData(netmsg_DataHeader* pHeader);
	//重置心跳
	void ResetDTHeart();
	//检查心跳
	bool CheckHeart(long long dt);
	//检测定时发送
	int CheckSend(long long dt);
public:
	void SetServerID(int serverID);
private:
	//重置发送计时
	void ResetDTSend();
	//立刻发送发送缓冲区数据
	int FlushSendBuf();
	//关闭当前客户端的 socket
	void Close();
private:
	SOCKET _cSock = INVALID_SOCKET;
	//接收消息缓冲区
	char _szRecvBuf[RECV_BUF_SIZE] = {};
	//接收消息缓冲区位置
	int _lastRecvPos = 0;
	//发送缓冲区
	char _szSendBuf[SEND_BUF_SIZE] = {};
	//发送缓冲区位置
	int _lastSendPos = 0;
	//心跳计时
	long long _dtHeart = 0;
	//定时发送计时
	long long _dtSend = 0;
	int _id = -1;
	int _serverID = -1;
};

CELLClient::CELLClient(SOCKET cSock)
{
	static int n = 1;
	_id = n++;
	this->_cSock = cSock;
}

CELLClient::~CELLClient()
{
	printf("server=%d CELLClient %d::~CELLClient\n", _serverID, _id);
	Close();
}

inline SOCKET CELLClient::GetSocketfd()
{
	return _cSock;
}

inline int CELLClient::GetLastRecvPos()
{
	return _lastRecvPos;
}

inline void CELLClient::SetLastRecvPos(int newPos)
{
	_lastRecvPos = newPos;
}

inline char * CELLClient::RecvBuf()
{
	return _szRecvBuf;
}
inline void CELLClient::Close()
{
	//printf("server=%d CELLClient %d::Close start\n",_serverID, _id);
	if (_cSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_cSock);
#else
		close(_cSock);
#endif
		_cSock = INVALID_SOCKET;
	}
	//printf("server=%d CELLClient %d::Close end\n", _serverID, _id);
}
inline int CELLClient::SendData(netmsg_DataHeader * pHeader)
{
	int ret = SOCKET_ERROR;
	int nSendLen = pHeader->dataLength;
	const char* pSendData = (const char*)pHeader;

	while (true)
	{
		if ((_lastSendPos + nSendLen) >= SEND_BUF_SIZE)
		{
			int nCopyLen = SEND_BUF_SIZE - _lastSendPos;
			if (nCopyLen > 0)
			{
				//复制数据
				memcpy(_szSendBuf + _lastSendPos, pSendData, nCopyLen);
				//修正位置
				pSendData += nCopyLen;
				nSendLen -= nCopyLen;
			}

			//发送数据
			ret = send(_cSock, _szSendBuf, SEND_BUF_SIZE, 0);
			//清空缓冲区
			_lastSendPos = 0;
			ResetDTSend();
			if (SOCKET_ERROR == ret)
			{
				printf("sock=%d sendData fail.\n", (int)_cSock);
				return ret;
			}
		}
		else {
			//消息没有达到缓冲区最大，就只是复制数据
			memcpy(_szSendBuf + _lastSendPos, pSendData, nSendLen);
			_lastSendPos += nSendLen;
			ret = nSendLen;
			break;
		}
	}

	return ret;
}

inline void CELLClient::ResetDTHeart()
{
	_dtHeart = 0;
}

inline bool CELLClient::CheckHeart(long long dt)
{
	_dtHeart += dt;
	if (_dtHeart >= CLIENT_HEART_DEATH_TIME)
	{
		printf("checkHeart sock=%d, time=%lld\n", (int)_cSock, _dtHeart);
		return true;
	}
	return false;
}

inline void CELLClient::ResetDTSend()
{
	_dtSend = 0;
}

inline int CELLClient::CheckSend(long long dt)
{
	_dtSend += dt;
	if (_dtSend >= FLUSH_SEND_BUF_2_CLIENT_TIME)
	{
		//printf("CheckSend sock=%d, time=%lld\n", (int)_cSock, _dtSend);
		FlushSendBuf();
		ResetDTSend();
		return true;
	}
	return false;
}

inline void CELLClient::SetServerID(int serverID)
{
	_serverID = serverID;
}

inline int CELLClient::FlushSendBuf()
{
	int ret = 0;
	if (_lastSendPos > 0)
	{
		//发送数据
		ret = send(_cSock, _szSendBuf, _lastSendPos, 0);
		//清空缓冲区
		_lastSendPos = 0;
		if (SOCKET_ERROR == ret)
		{
			printf("sock=%d FlushSendBuf fail.\n", (int)_cSock);			
		}		
	}

	return ret;
}

#endif // _CELL_CLIENT_HPP_
