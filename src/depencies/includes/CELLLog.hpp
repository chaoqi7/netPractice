#ifndef _CELL_LOG_H_
#define _CELL_LOG_H_

#include <ctime>
#include "CELLTask.hpp"
#include <stdio.h>
#include <errno.h>

class CELLLog
{
public:
private:
	CELLLog();
	~CELLLog();
	static CELLLog &Instance();

public:
	static void setLogPath(const char *pName, const char *pMode, bool hasDate);
	//Info
	static void Info(const char *pStr)
	{
		Info("%s", pStr);
	}
	template <typename... Args>
	static void Info(const char *format, Args... args)
	{
		WriteLog("Info", format, args...);
	}
	//Debug
	static void Debug(const char *pStr)
	{
		Debug("%s", pStr);
	}
	template <typename... Args>
	static void Debug(const char *format, Args... args)
	{
		WriteLog("Debug", format, args...);
	}
	//Warning
	static void Warning(const char *pStr)
	{
		Warning("%s", pStr);
	}
	template <typename... Args>
	static void Warning(const char *format, Args... args)
	{
		WriteLog("Warning", format, args...);
	}
	//Error
	static void Error(const char *pStr)
	{
		Error("%s", pStr);
	}
	template <typename... Args>
	static void Error(const char *format, Args... args)
	{
		WriteLog("Error", format, args...);
	}
	//PError
	static void PError(const char *pStr)
	{
		PError("%s", pStr);
	}
	template <typename... Args>
	static void PError(const char *format, Args... args)
	{
#ifdef _WIN32
		auto errCode = GetLastError();
		Instance()._taskServer.AddTask([=]() {
			char errMsg[256] = {};
			FormatMessageA(
				FORMAT_MESSAGE_FROM_SYSTEM,
				NULL,
				errCode,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				errMsg,
				256,
				NULL);
			WriteLogReal(true, "PError", format, args...);
			WriteLogReal(false, "PError", "errno=%d, error msg=%s", errCode, errMsg);
		});
#else
		auto errCode = errno;
		Instance()._taskServer.AddTask([=]() {
			WriteLogReal(true, "PError", format, args...);
			WriteLogReal(true, "PError", "errno=%d, error msg=%s", errCode, strerror(errCode));
		});
#endif
	}

private:
	template <typename... Args>
	static void WriteLog(const char *szType, const char *pFormat, Args... args)
	{
		auto pLog = &Instance();
		pLog->_taskServer.AddTask([=]() {
			WriteLogReal(true, szType, pFormat, args...);
		});
	}

	template <typename... Args>
	static void WriteLogReal(bool br, const char *szType, const char *pFormat, Args... args)
	{
		auto pLog = &Instance();
		auto t = std::chrono::system_clock::now();
		auto now = std::chrono::system_clock::to_time_t(t);
		std::tm *tNow = std::localtime(&now);
		//写入日志类型，时间
		fprintf(pLog->_pLogFile, "%s [%d-%02d-%02d %02d:%02d:%02d] ",
				szType, tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday,
				tNow->tm_hour, tNow->tm_min, tNow->tm_sec);
		if (szType)
		{
			fprintf(pLog->_pLogFile, "%s ", szType);
		}
		fprintf(pLog->_pLogFile, pFormat, args...);
		if (br)
		{
			fprintf(pLog->_pLogFile, "%s", "\n");
		}
		fflush(pLog->_pLogFile);
		//写日志到 Console
		if (szType)
		{
			printf("%s ", szType);
		}
		printf(pFormat, args...);
		if (br)
		{
			printf("%s", "\n");
		}
	}

private:
	CellTaskServer _taskServer;
	FILE *_pLogFile = nullptr;
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

inline CELLLog &CELLLog::Instance()
{
	static CELLLog obj;
	return obj;
}

inline void CELLLog::setLogPath(const char *pName, const char *pMode, bool hasDate)
{
	auto pLog = &Instance();
	if (pLog->_pLogFile)
	{
		Info("CELLLog setLogPath _pLogFile != nullptr");
		fclose(pLog->_pLogFile);
		pLog->_pLogFile = nullptr;
	}

	static char pPath[256] = {};
	if (hasDate)
	{
		auto t = std::chrono::system_clock::now();
		auto now = std::chrono::system_clock::to_time_t(t);
		std::tm *tNow = std::localtime(&now);

		sprintf(pPath, "%s_%d-%02d-%02d_%02d-%02d-%02d.txt", pName,
				tNow->tm_year + 1900, tNow->tm_mon + 1, tNow->tm_mday,
				tNow->tm_hour, tNow->tm_min, tNow->tm_sec);
	}
	else
	{
		sprintf(pPath, "%s.txt", pName);
	}

	pLog->_pLogFile = fopen(pPath, pMode);
	if (pLog->_pLogFile)
	{
		Info("CELLLog setLogPath<%s, %s> success.", pPath, pMode);
	}
	else
	{
		Info("CELLLog setLogPath<%s, %s> failed.", pPath, pMode);
	}
}

#ifdef _DEBUG
#ifndef CELLLog_Debug
#define CELLLog_Debug(...) CELLLog::Debug(__VA_ARGS__)
#endif
#else
#ifndef CELLLog_Debug
#define CELLLog_Debug(...)
#endif
#endif

#ifndef CELLLog_Info
#define CELLLog_Info(...) CELLLog::Info(__VA_ARGS__)
#endif
#ifndef CELLLog_Warnning
#define CELLLog_Warnning(...) CELLLog::Warning(__VA_ARGS__)
#endif
#ifndef CELLLog_Error
#define CELLLog_Error(...) CELLLog::Error(__VA_ARGS__)
#endif
#ifndef CELLLog_PError
#define CELLLog_PError(...) CELLLog::PError(__VA_ARGS__)
#endif
#endif // _CELL_LOG_H_
