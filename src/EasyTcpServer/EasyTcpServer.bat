::
@echo off
::设置IP
set cmd="strIP=any"
::设置端口
set cmd=%cmd% nPort=4567
::线程数
set cmd=%cmd% nThread=1
::能接收的最大客户端数量
set cmd=%cmd% nMaxClient=10240
::客户端发送缓冲区大小（字节）
set cmd=%cmd% nSendBuffSize=81920
::客户端接收缓冲区大小（字节）
set cmd=%cmd% nRecvBuffSize=81920
::收到消息后将返回应答消息
::提示发送缓冲区已写满
::当出现 sendfull 提示时，表示当次消息被丢弃
set cmd=%cmd% -sendfull
::检查接收到的客户端消息ID是否连续
set cmd=%cmd% -checkMsgID
::是否回消息
set cmd=%cmd% -sendback

EasyTcpServer %cmd%


@pause