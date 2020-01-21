#ifndef _CELL_HPP_
#define _CELL_HPP_


#ifdef _WIN32
//windows
#define FD_SETSIZE      10240
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996)
#else
#ifdef __APPLE__
#define _DARWIN_UNLIMITED_SELECT
#endif
//linux and osx
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)
#define SOCKET int
#endif // _WIN32

#include <stdio.h>

#include "NetMsg.h"
#include "CELLLog.hpp"



#endif