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
	CMD_LOGIN,
	CMD_S2C_LOGIN,
	CMD_LOGOUT,
	CMD_S2C_LOGOUT,
	CMD_S2C_NEW_USER_JOIN,
	CMD_C2S_HEART,
	CMD_S2C_HEART,
	CMD_S2C_ERROR,
};

struct netmsg_DataHeader
{	
	netmsg_DataHeader()
	{
		this->dataLength = sizeof(netmsg_DataHeader);
		this->cmd = CMD_S2C_ERROR;
	}
	short dataLength;
	short cmd;	
};

//登录相关
struct netmsg_C2S_Login : public netmsg_DataHeader
{
	netmsg_C2S_Login()
	{
		this->dataLength = sizeof(netmsg_C2S_Login);
		this->cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
	char data[56];
};

struct netmsg_S2C_Login : public netmsg_DataHeader
{
	netmsg_S2C_Login()
	{
		this->dataLength = sizeof(netmsg_S2C_Login);
		this->cmd = CMD_S2C_LOGIN;
		this->result = 0;
	}
	int result;
	char data[96];
};

//登出相关
struct netmsg_C2S_Logout : public netmsg_DataHeader
{
	netmsg_C2S_Logout()
	{
		this->dataLength = sizeof(netmsg_C2S_Logout);
		this->cmd = CMD_LOGOUT;
	}
	char userName[32];
};

struct netmsg_S2C_Logout : public netmsg_DataHeader
{
	netmsg_S2C_Logout()
	{
		this->dataLength = sizeof(netmsg_S2C_Logout);
		this->cmd = CMD_S2C_LOGOUT;
		this->result = 0;
	}
	int result;
};

struct netmsg_S2C_NewUserJoin : public netmsg_DataHeader
{
	netmsg_S2C_NewUserJoin()
	{
		this->dataLength = sizeof(netmsg_S2C_NewUserJoin);
		this->cmd = CMD_S2C_NEW_USER_JOIN;
		this->sock = 0;
	}
	int sock;
};

struct netmsg_C2S_Heart : public netmsg_DataHeader
{
	netmsg_C2S_Heart()
	{
		this->dataLength = sizeof(netmsg_C2S_Heart);
		this->cmd = CMD_C2S_HEART;
	}
};

struct netmsg_S2C_Heart : public netmsg_DataHeader
{
	netmsg_S2C_Heart()
	{
		this->dataLength = sizeof(netmsg_S2C_Heart);
		this->cmd = CMD_S2C_HEART;
	}
};

#endif // _NET_MSG_H_
