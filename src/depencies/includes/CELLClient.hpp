#ifndef _CELL_CLIENT_HPP_
#define _CELL_CLIENT_HPP_
//心跳检测死亡倒计时时间
#define CLIENT_HEART_DEATH_TIME 60 * 1000
//定时把发送缓冲区数据发送到客户端时间(200毫秒是最小值)
#define FLUSH_SEND_BUF_2_CLIENT_TIME 200

#include "CELL.hpp"
#include "CELLSendBuffer.hpp"
#include "CELLRecvBuffer.hpp"

class CELLClient
{
public:
	CELLClient(SOCKET cSock, int sendSize, int recvSize);
	~CELLClient();
	//获取当前客户端的 socket
	SOCKET getSocketfd();
	//发送消息
	int SendData(netmsg_DataHeader* pHeader);
	//立即发送数据
	int SendDataReal();
	//读取数据
	int ReadData();
	//是否有消息
	bool HasMsg();
	//获取第一条消息
	netmsg_DataHeader* FrontMsg();
	//删除第一条消息
	void PopFrontMsg();

	//重置心跳
	void ResetDTHeart();
	//检查心跳
	bool CheckHeart(long long dt);
	//检测定时发送
	bool CheckSend(long long dt);
public:
	void SetServerID(int serverID);
private:
	//重置发送计时
	void ResetDTSend();
	//关闭当前客户端的 socket
	void Close();
private:
	SOCKET _cSock = INVALID_SOCKET;
	//缓冲区的控制根据业务需求的差异而调整
	//接收消息缓冲区
	CELLRecvBuffer _recvBuf;
	//发送消息缓冲区
	CELLSendBuffer _sendBuf;
	//心跳计时
	long long _dtHeart = 0;
	//定时发送计时
	long long _dtSend = 0;
	int _id = -1;
	int _serverID = -1;
};

CELLClient::CELLClient(SOCKET cSock, int sendSize, int recvSize)
	:_sendBuf(sendSize),_recvBuf(recvSize)
{
	static int n = 1;
	_id = n++;
	this->_cSock = cSock;
	ResetDTHeart();
	ResetDTSend();
}

CELLClient::~CELLClient()
{
	printf("server=%d CELLClient %d::~CELLClient\n", _serverID, _id);
	Close();
}

inline SOCKET CELLClient::getSocketfd()
{
	return _cSock;
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

inline void CELLClient::SetServerID(int serverID)
{
	_serverID = serverID;
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

inline bool CELLClient::CheckSend(long long dt)
{
	_dtSend += dt;
	if (_dtSend >= FLUSH_SEND_BUF_2_CLIENT_TIME)
	{
		//printf("CheckSend sock=%d, time=%lld\n", (int)_cSock, _dtSend);
		SendDataReal();
		ResetDTSend();
		return true;
	}
	return false;
}


inline int CELLClient::SendData(netmsg_DataHeader * pHeader)
{
	return _sendBuf.WriteData(pHeader);
}

inline int CELLClient::SendDataReal()
{
	ResetDTSend();
	return _sendBuf.Write2Socket(_cSock);
}

inline int CELLClient::ReadData()
{
	return _recvBuf.read4socket(_cSock);
}

inline bool CELLClient::HasMsg()
{
	return _recvBuf.hasMsg();
}

inline netmsg_DataHeader * CELLClient::FrontMsg()
{
	return _recvBuf.frontMsg();
}

inline void CELLClient::PopFrontMsg()
{
	_recvBuf.popFrontMsg();
}


#endif // _CELL_CLIENT_HPP_
