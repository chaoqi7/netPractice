#ifndef _EASY_TCP_CLIENT_HPP_
#define _EASY_TCP_CLIENT_HPP_

#include "CELL.hpp"
#include "CELLClient.hpp"
#include "CELLNetWork.hpp"
#include "CELLWriteStream.hpp"

class EasyTcpClient
{
public:
	EasyTcpClient();
	virtual ~EasyTcpClient();
	//连接远程服务器
	int Connect(const char* ip, unsigned short port);
	//初始化 socket
	SOCKET InitSocket(int sendSize, int recvSize);
	//关闭连接
	void Close();	
	//循环执行任务（当前使用select)
	bool OnRun(int microseconds);
	//是否运行
	bool IsRun();
	//发送消息
	int SendData(netmsg_DataHeader* pHeader);
	int SendData(const char* pData, int nLen);
private:
	//接收数据
	int RecvData();
	//处理消息
	virtual void OnNetMsg(netmsg_DataHeader* pHeader) = 0;
protected:
	CELLClient* _pClient = nullptr;
	bool _bConnect = false;
};

EasyTcpClient::EasyTcpClient()
{
	_bConnect = false;
}

EasyTcpClient::~EasyTcpClient()
{
	Close();
}

inline SOCKET EasyTcpClient::InitSocket(int sendSize, int recvSize)
{
	if (IsRun())
	{
		CELLLog_Info("InitSocket Close old socket=%d.", (int)_pClient->getSocketfd());
		Close();
	}
	
	CELLNetWork::Init();

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sock)
	{
		CELLLog_Info("create socket fail.");
	}
	else {
		//CELLLog_Debug("创建 socket=%d 成功.", (int)sock);
		_pClient = new CELLClient(sock, sendSize, recvSize);
	}
	return sock;
}

inline int EasyTcpClient::Connect(const char * ip, unsigned short port)
{
	if (!_pClient)
	{
		if (INVALID_SOCKET == InitSocket(SEND_BUF_SIZE, RECV_BUF_SIZE))
		{
			return SOCKET_ERROR;
		}
	}

	sockaddr_in _sin = {};
	_sin.sin_family = AF_INET;
	_sin.sin_port = htons(port);
#ifdef _WIN32
	_sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
	_sin.sin_addr.s_addr = inet_addr(ip);
#endif
	int ret = connect(_pClient->getSocketfd(), (sockaddr*)&_sin, sizeof(sockaddr_in));
	if (SOCKET_ERROR == ret)
	{
		CELLLog_Error("<sockt=%d> connect <%s:%d> failed.", 
			(int)_pClient->getSocketfd(), ip, port);
		return -1;
	}
	else {
		//CELLLog_Info("<sockt=%d> connect <%s:%d> success.", (int)_sock, ip, port);
		_bConnect = true;
	}
	return ret;
}

inline void EasyTcpClient::Close()
{
	if (_pClient)
	{
		delete _pClient;
		_pClient = nullptr;
	}
	_bConnect = false;
}

inline bool EasyTcpClient::IsRun()
{
	return _bConnect && _pClient;
}

inline bool EasyTcpClient::OnRun(int microseconds)
{
	if (IsRun())
	{
		SOCKET cSock = _pClient->getSocketfd();

		fd_set fdRead;
		FD_ZERO(&fdRead);
		FD_SET(cSock, &fdRead);

		fd_set fdWrite;			
		FD_ZERO(&fdWrite);		

		timeval t = { 0, microseconds };
		int ret = 0;
		if (_pClient->NeedWrite())
		{			
			FD_SET(cSock, &fdWrite);
			ret = select((int)cSock + 1, &fdRead, &fdWrite, nullptr, &t);
		}
		else {
			ret = select((int)cSock + 1, &fdRead, nullptr, nullptr, &t);
		}

		if (ret == SOCKET_ERROR)
		{
			CELLLog_Error("EasyTcpClient::OnRun select error.");
			Close();
			return false;
		}
		else if (ret == 0) {
			//CELLLog_Info("select time out.");
			//continue;
		}

		if (FD_ISSET(cSock, &fdRead))
		{
			if (-1 == RecvData())
			{
				CELLLog_Error("<sockt=%d> EasyTcpClient::OnRun RecvData error.", (int)cSock);
				Close();
				return false;
			}
		}

		if (FD_ISSET(cSock, &fdWrite))
		{
			if (-1 == _pClient->SendDataReal())
			{
				CELLLog_Error("<sockt=%d> EasyTcpClient::OnRun SendDataReal error.", (int)cSock);
				Close();
				return false;
			}
		}
		return true;
	}
	return false;
}

inline int EasyTcpClient::SendData(netmsg_DataHeader * pHeader)
{
	return SendData((const char*)pHeader, pHeader->dataLength);
}

inline int EasyTcpClient::SendData(const char * pData, int nLen)
{
	if (IsRun())
	{
		return _pClient->SendData(pData, nLen);
	}
	return 0;
}

inline int EasyTcpClient::RecvData()
{
	if (IsRun())
	{
		int nLen = _pClient->ReadData();
		if (nLen > 0)
		{
			//是否有消息
			while (_pClient->HasMsg())
			{
				OnNetMsg(_pClient->FrontMsg());
				_pClient->PopFrontMsg();
			}
		}

		return nLen;
	}
	return 0;
}

#endif //_EASY_TCP_CLIENT_HPP_