
#ifndef _CELL_CONFIG_HPP_
#define _CELL_CONFIG_HPP_

#include "CELLLog.hpp"
#include <map>

class CELLConfig
{
private:
	CELLConfig();
	~CELLConfig();
public:
	static CELLConfig& Instance()
	{
		static CELLConfig obj;
		return obj;
	}

	void Init(int argc, char** argv)
	{
		_appName = argv[0];
		for (int n = 0; n < argc; n++)
		{
			//CELLLog_Debug("%s", argv[n]);
			makeKV(argv[n]);
		}
	}

	const char* getStr(const char* argName, const char* def)
	{
		auto itr = _kv.find(argName);
		if (itr == _kv.end())
		{
			CELLLog_Error("CELLConfig::getStr, argName=%s", argName);
		}
		else {
			def = itr->second.c_str();
		}
		CELLLog_Info("CELLConfig::getStr %s=%s", argName, def);
		return def;
	}

	int getInt(const char* argName, int def)
	{
		auto itr = _kv.find(argName);
		if (itr == _kv.end())
		{
			CELLLog_Error("CELLConfig::getInt, argName=%s", argName);
		}
		else {
			def = atoi(itr->second.c_str());
		}
		CELLLog_Info("CELLConfig::getInt %s=%d", argName, def);
		return def;
	}

	bool hasKey(const char* key)
	{
		auto itr = _kv.find(key);
		return itr != _kv.end();
	}

private:
	void makeKV(char* cmd)
	{
		char* val = strchr(cmd, '=');
		if (val) {
			val[0] = '\0';
			val++;
			_kv[cmd] = val;
			//CELLLog_Debug("CELLConfig::makeKV <%s, %s>", cmd, val);
		}
		else {
			_kv[cmd] = cmd;
			//CELLLog_Debug("CELLConfig::makeKV <%s>", cmd);
		}
	}
private:
	char* _appName = nullptr;
	std::map<std::string, std::string> _kv;
};

CELLConfig::CELLConfig()
{
}

CELLConfig::~CELLConfig()
{
}

#endif // _CELL_CONFIG_HPP_