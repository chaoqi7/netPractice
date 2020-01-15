
::
@echo off
::设置IP
set strIP=192.168.3.61
::设置端口
set nPort=4567
::工作线程数量
set nThread=1
::每个工作线程，创建客户端数量
set nClient=8
::::::数据会先写入发送缓冲区
::::::等待 socket 可写时才实际发送
::每个客户端在 nSendSleep（毫秒）时间内
::最大可写入 nMsg 条 Login 消息
::每条消息 100 字节
set nMsg=100
::
set nSendSleep=1000
::客户端发送缓冲区大小（字节）
set nSendBuffSize=81920
::客户端接收缓冲区大小（字节）
set nRecvBuffSize=81920
::检查接收到的服务端消息ID是否连续
set -checkMsgID


set cmd=strIP=%strIP%
set cmd=%cmd% nPort=%nPort%
set cmd=%cmd% nThread=%nThread%
set cmd=%cmd% nClient=%nClient%
set cmd=%cmd% nMsg=%nMsg%
set cmd=%cmd% nSendSleep=%nSendSleep%
set cmd=%cmd% nSendBuffSize=%nSendBuffSize%
set cmd=%cmd% nRecvBuffSize=%nRecvBuffSize%
set cmd=%cmd% -checkMsgID=%-checkMsgID%

EasyTcpClient %cmd%


@pause