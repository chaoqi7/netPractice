#ifndef _CELL_LOG_H_
#define _CELL_LOG_H_

#include <ctime>
#include "CELLTask.hpp"
#include <stdio.h>

class CELLLog
{
private:
	CELLLog();
	~CELLLog();
	static CELLLog& Instance();
public:
	static void setLogPath(const char* pPath, const char* pMode);
	//Info
	static void Info(const char* pStr)
	{
		WriteLog("INFO", "%s", pStr);
	}
	template<typename ...Args>
	static void Info(const char* format, Args ... args)
	{
		WriteLog("INFO", format, args...);
	}
	//Debug
	static void Debug(const char* pStr)
	{
		WriteLog("DEBUG", "%s", pStr);
	}
	template<typename ...Args>
	static void Debug(const char* format, Args ... args)
	{
		WriteLog("DEBUG", format, args...);
	}
	//Warning
	static void Warning(const char* pStr)
	{
		WriteLog("WARNING", "%s", pStr);
	}
	template<typename ...Args>
	static void Warning(const char* format, Args ... args)
	{
		WriteLog("WARNING", format, args...);
	}
	//Error
	static void Error(const char* pStr)
	{
		WriteLog("ERROR", "%s", pStr);
	}
	template<typename ...Args>
	static void Error(const char* format, Args ... args)
	{
		WriteLog("ERROR", format, args...);
	}
private:
	template<typename ...Args>
	static void WriteLog(const char* szType, const char* pFormat, Args ... args)
	{
		auto pLog = &Instance();
		pLog->_taskServer.AddTask([=]() {
			auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			std::tm* tNow = std::gmtime(&now);
			//获取时区
			std::time_t local = std::mktime(std::localtime(&now));
			std::time_t gmt = std::mktime(std::gmtime(&now));			
			auto timezone = static_cast<int> ((local - gmt)/(60*60));
			//写入日志类型，时间
			fprintf(pLog->_pLogFile, "%s [%d-%02d-%02d %02d:%02d:%02d] ",
				szType, tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday, 
				tNow->tm_hour + timezone, tNow->tm_min, tNow->tm_sec);
			fprintf(pLog->_pLogFile, pFormat, args...);
			fflush(pLog->_pLogFile);
		});
		printf(pFormat, args...);
	}
private:
	CellTaskServer _taskServer;
	FILE* _pLogFile = nullptr;
};

CELLLog::CELLLog()
{
	_taskServer.Start(100);
}

CELLLog::~CELLLog()
{
	_taskServer.Close();
	if (_pLogFile)
	{
		Info("CELLLog ~CELLLog close log file.");
		fclose(_pLogFile);
		_pLogFile = nullptr;
	}	
}

inline CELLLog & CELLLog::Instance()
{
	static CELLLog obj;
	return obj;
}

inline void CELLLog::setLogPath(const char * pPath, const char * pMode)
{
	auto pLog = &Instance();
	if (pLog->_pLogFile)
	{
		Info("CELLLog setLogPath _pLogFile != nullptr");
		fclose(pLog->_pLogFile);
		pLog->_pLogFile = nullptr;
	}

	pLog->_pLogFile = fopen(pPath, pMode);
	if (pLog->_pLogFile)
	{
		Info("CELLLog setLogPath<%s, %s> success.\n", pPath, pMode);
	}
	else {
		Info("CELLLog setLogPath<%s, %s> failed.\n", pPath, pMode);
	}
}

#endif // _CELL_LOG_H_


