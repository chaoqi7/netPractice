::
@echo off
::����IP
set strIP=any
::���ö˿�
set nPort=4567

set nThread=1

set nClient=8


set cmd=strIP=%strIP%
set cmd=%cmd% nPort=%nPort%
set cmd=%cmd% nThread=%nThread%
set cmd=%cmd% nClient=%nClient%

EasyTcpServer %cmd%


@pause