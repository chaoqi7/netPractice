#ifndef _NET_MSG_H_
#define _NET_MSG_H_

//接收缓冲区大小
#ifndef RECV_BUF_SIZE
#define RECV_BUF_SIZE 4096
#endif // RECV_BUF_SIZE
//发送缓冲区大小
#ifndef SEND_BUF_SIZE
#define SEND_BUF_SIZE 409600
#endif // SEND_BUF_SIZE

enum CMD
{
	CMD_C2S_LOGIN,
	CMD_S2C_LOGIN,
	CMD_C2S_LOGOUT,
	CMD_S2C_LOGOUT,
	CMD_S2C_NEW_USER_JOIN,
	CMD_C2S_HEART,
	CMD_S2C_HEART,
	CMD_C2S_STREAM,
	CMD_S2C_STREAM,
	CMD_S2C_ERROR,
};

struct netmsg_DataHeader
{	
	netmsg_DataHeader()
	{
		this->dataLength = sizeof(netmsg_DataHeader);
		this->cmd = CMD_S2C_ERROR;
	}
	unsigned short dataLength;
	unsigned short cmd;
};

//登录相关
struct netmsg_Login : public netmsg_DataHeader
{
	netmsg_Login()
	{
		this->dataLength = sizeof(netmsg_Login);
		this->cmd = CMD_C2S_LOGIN;
	}
	char userName[32];
	char passWord[32];
	char data[56];
	int msgID;
};

struct netmsg_LoginR : public netmsg_DataHeader
{
	netmsg_LoginR()
	{
		this->dataLength = sizeof(netmsg_LoginR);
		this->cmd = CMD_S2C_LOGIN;
		this->result = 0;
	}
	int result;
	char data[96];
	int msgID;
};

//登出相关
struct netmsg_Logout : public netmsg_DataHeader
{
	netmsg_Logout()
	{
		this->dataLength = sizeof(netmsg_Logout);
		this->cmd = CMD_C2S_LOGOUT;
	}
	char userName[32];
};

struct netmsg_LogoutR : public netmsg_DataHeader
{
	netmsg_LogoutR()
	{
		this->dataLength = sizeof(netmsg_LogoutR);
		this->cmd = CMD_S2C_LOGOUT;
		this->result = 0;
	}
	int result;
};

struct netmsg_NewUserJoin : public netmsg_DataHeader
{
	netmsg_NewUserJoin()
	{
		this->dataLength = sizeof(netmsg_NewUserJoin);
		this->cmd = CMD_S2C_NEW_USER_JOIN;
		this->sock = 0;
	}
	int sock;
};

struct netmsg_Heart : public netmsg_DataHeader
{
	netmsg_Heart()
	{
		this->dataLength = sizeof(netmsg_Heart);
		this->cmd = CMD_C2S_HEART;
	}
};

struct netmsg_HeartR : public netmsg_DataHeader
{
	netmsg_HeartR()
	{
		this->dataLength = sizeof(netmsg_HeartR);
		this->cmd = CMD_S2C_HEART;
	}
};

#endif // _NET_MSG_H_
