#ifndef _NET_MSG_H_
#define _NET_MSG_H_

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
		this->sock = INVALID_SOCKET;
	}
	int sock;
};

#endif // _NET_MSG_H_
