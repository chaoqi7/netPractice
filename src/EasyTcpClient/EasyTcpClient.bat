
::
@echo off
::����IP
set strIP=127.0.0.1
::���ö˿�
set nPort=4567
::�����߳�����
set nThread=1
::ÿ�������̣߳������ͻ�������
set nClient=10000
::::::���ݻ���д�뷢�ͻ�����
::::::�ȴ� socket ��дʱ��ʵ�ʷ���
::ÿ���ͻ����� nSendSleep�����룩ʱ����
::����д�� nMsg �� Login ��Ϣ
::ÿ����Ϣ 100 �ֽ�
set nMsg=100
::
set nSendSleep=1
::�ͻ��˷��ͻ�������С���ֽڣ�
set nSendBuffSize=81920
::�ͻ��˽��ջ�������С���ֽڣ�
set nRecvBuffSize=81920
::�����յ��ķ������ϢID�Ƿ�����
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