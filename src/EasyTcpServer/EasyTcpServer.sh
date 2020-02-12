#设置IP
strIP="any"
#设置端口
nPort=4567

nThread=1

nClient=10240

nSendBuffSize=81920


nRecvBuffSize=81920


-sendfull="-sendfull"

-checkMsgID="-checkMsgID"

-sendback="-sendback"

EasyTcpServer $strIP $nPort $nThread $nClient $nSendBuffSize $nRecvBuffSize $-sendfull $-checkMsgID $-sendback


read -p "please input any key to continue..."