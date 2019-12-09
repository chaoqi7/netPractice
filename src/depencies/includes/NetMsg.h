#ifndef _NET_MSG_H_
#define _NET_MSG_H_

//接收缓冲区大小
#ifndef RECV_BUF_SIZE
#define RECV_BUF_SIZE 10240 * 5
#endif // RECV_BUF_SIZE
//发送缓冲区大小
#ifndef SEND_BUF_SIZE
#define SEND_BUF_SIZE 10240 * 5
#endif // SEND_BUF_SIZE

enum CMD
{
	CMD_LOGIN,
	CMD_LOGIN_RESULT,
	CMD_LOGINOUT,
	CMD_LOGINOUT_RESULT,
	CMD_NEW_USER_JOIN,
	CMD_ERROR,
};

struct DataHeader
{	
	DataHeader()
	{
		this->dataLength = sizeof(DataHeader);
		this->cmd = CMD_ERROR;
	}
	short dataLength;
	short cmd;	
};

//登录相关
struct Login : public DataHeader
{
	Login()
	{
		this->dataLength = sizeof(Login);
		this->cmd = CMD_LOGIN;
	}
	char userName[32];
	char passWord[32];
	char data[56];
};

struct LoginResult : public DataHeader
{
	LoginResult()
	{
		this->dataLength = sizeof(LoginResult);
		this->cmd = CMD_LOGIN_RESULT;
		this->result = 0;
	}
	int result;
	char data[96];
};

//登出相关
struct Logout : public DataHeader
{
	Logout()
	{
		this->dataLength = sizeof(Logout);
		this->cmd = CMD_LOGINOUT;
	}
	char userName[32];
};

struct LogoutResult : public DataHeader
{
	LogoutResult()
	{
		this->dataLength = sizeof(LogoutResult);
		this->cmd = CMD_LOGINOUT_RESULT;
		this->result = 0;
	}
	int result;
};

struct NewUserJoin : public DataHeader
{
	NewUserJoin()
	{
		this->dataLength = sizeof(NewUserJoin);
		this->cmd = CMD_NEW_USER_JOIN;
		this->sock = 0;
	}
	int sock;
};

#endif // _NET_MSG_H_
