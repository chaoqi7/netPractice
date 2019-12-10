#ifndef _CELL_CLIENT_HPP_
#define _CELL_CLIENT_HPP_

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
	//关闭当前客户端的 socket
	void Close();
	//发送消息
	int SendData(netmsg_DataHeader* pHeader);
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
};

CELLClient::CELLClient(SOCKET cSock)
{
	this->_cSock = cSock;
}

CELLClient::~CELLClient()
{
	_cSock = INVALID_SOCKET;
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
	if (_cSock != INVALID_SOCKET)
	{
#ifdef _WIN32
		closesocket(_cSock);
#else
		close(_cSock);
#endif
		_cSock = INVALID_SOCKET;
	}
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

#endif // _CELL_CLIENT_HPP_
