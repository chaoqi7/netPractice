::
@echo off
::����IP
set cmd="strIP=any"
::���ö˿�
set cmd=%cmd% nPort=4567
::�߳���
set cmd=%cmd% nThread=1
::�ܽ��յ����ͻ�������
set cmd=%cmd% nMaxClient=10240
::�ͻ��˷��ͻ�������С���ֽڣ�
set cmd=%cmd% nSendBuffSize=81920
::�ͻ��˽��ջ�������С���ֽڣ�
set cmd=%cmd% nRecvBuffSize=81920
::�յ���Ϣ�󽫷���Ӧ����Ϣ
::��ʾ���ͻ�������д��
::������ sendfull ��ʾʱ����ʾ������Ϣ������
set cmd=%cmd% -sendfull
::�����յ��Ŀͻ�����ϢID�Ƿ�����
set cmd=%cmd% -checkMsgID
::�Ƿ����Ϣ
set cmd=%cmd% -sendback

EasyTcpServer %cmd%


@pause