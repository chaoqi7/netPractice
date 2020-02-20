#ifndef _CELL_EPOLL_H_
#define _CELL_EPOLL_H_

#ifdef __linux__

#include "CELL.hpp"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

#define EPOLL_ERROR (-1)

class CELLEpoll
{
public:
    void create(int maxsize)
    {
        _maxEvent = maxsize;
        _epfd = epoll_create(_maxEvent);
        if (_epfd == EPOLL_ERROR)
        {
            CELLLog_PError("epoll_create");
        }
        _pEvents = new epoll_event[_maxEvent];
    }
    //增加对指定 socket 的事件监听
    int ctl(int op, SOCKET sock, uint32_t events)
    {
        epoll_event event = {};
        event.data.fd = sock;
        event.events = events;
        int ret = epoll_ctl(_epfd, op, sock, &event);
        if (ret == EPOLL_ERROR)
        {
            CELLLog_PError("epoll_ctl(epfd=%d, op=%d, sock=%d) fail.\n", _epfd, op, sock);
        }
        return ret;
    }

    int ctl(int op, CELLClient *pClient, uint32_t events)
    {
        epoll_event event = {};
        event.data.ptr = pClient;
        event.events = events;
        SOCKET curSock = pClient->socketfd();
        int ret = epoll_ctl(_epfd, op, curSock, &event);
        if (ret == EPOLL_ERROR)
        {
            CELLLog_PError("epoll_ctl(epfd=%d, op=%d, sock=%d) fail.\n", _epfd, op, curSock);
        }
        return ret;
    }
    /*
    timeout        
    -1: 一直阻塞
    0: 马上返回
    >0: 具体的毫秒
    */
    //timeout in millseconds.
    int wait(int timeout)
    {
        int ret = epoll_wait(_epfd, _pEvents, _maxEvent, timeout);
        if (ret == EPOLL_ERROR)
        {
            CELLLog_PError("epoll_wait");
        }
        return ret;
    }

    epoll_event *events()
    {
        return _pEvents;
    }

    ~CELLEpoll()
    {
        destory();
    }

    void destory()
    {
        close(_epfd);
        if (_pEvents)
        {
            delete[] _pEvents;
            _pEvents = nullptr;
        }
    }

private:
    epoll_event *_pEvents = nullptr;
    // create epfd.
    int _maxEvent = 256;
    int _epfd = -1;
};

#endif // __linux__

#endif // _CELL_EPOLL_H_
