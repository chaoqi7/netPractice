#ifndef _I_NET_EVENT_H_
#define _I_NET_EVENT_H_

#include "NetMsg.h"

class CELLClient;
class CellServer;

class INetEvent
{
public:
	//网络断开
	virtual void OnNetLeave(CELLClient* pClient) = 0;
	//网络建立
	virtual void OnNetJoin(CELLClient* pClient) = 0;
	//调用 recv 
	virtual void OnNetRecv(CELLClient* pClient) = 0;
	//网络消息
	virtual void OnNetMsg(CellServer* pCellServer, CELLClient* pClient, netmsg_DataHeader* pHeader) = 0;
};

#endif // _I_NET_EVENT_H_
