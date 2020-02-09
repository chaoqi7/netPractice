#设置IP
strIP="any"
#设置端口
nPort=4567

nThread=1

nClient=1000

EasyTcpServer $strIP $nPort $nThread $nClient


read -p "please input any key to continue..."