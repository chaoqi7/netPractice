#设置IP
cmd="strIP=any"
#设置端口
cmd="$cmd nPort=4568"
#线程数
cmd="$cmd nThread=4"
#能接收的最大客户端数量
cmd="$cmd nMaxClient=10240"
#客户端发送缓冲区大小（字节）
cmd="$cmd nSendBuffSize=81920"
#客户端接收缓冲区大小（字节）
cmd="$cmd nRecvBuffSize=81920"
#收到消息后将返回应答消息
#提示发送缓冲区已写满
#当出现 sendfull 提示时，表示当次消息被丢弃
cmd="$cmd -sendfull"
#检查接收到的客户端消息ID是否连续
cmd="$cmd -checkMsgID"
#是否回消息
cmd="$cmd -sendback"

EasyTcpServer $cmd





read -p "please input any key to continue..."