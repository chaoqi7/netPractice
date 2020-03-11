# 日志系统


## 记录`普通级别`日志
- 实时写入日志
    ```cpp
    template <typename... Args>
	static void WriteLogReal(bool br, const char *szType, const char *pFormat, Args... args)
	{
		auto pLog = &Instance();
		//写入日志类型，时间
		fprintf(pLog->_pLogFile, "%s [%s] ", szType, CELLTime::getNowInStr().c_str());
		printf(pLog->_pLogFile, "%s ", szType);
		fprintf(pLog->_pLogFile, pFormat, args...);
		fprintf(pLog->_pLogFile, "%s", "\n");

		fflush(pLog->_pLogFile);
		//写日志到 Console
		printf("%s ", szType);
		printf(pFormat, args...);
		printf("%s", "\n");
	}

## 记录`系统级别错误`日志
- 通过操作系统 API
```cpp
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