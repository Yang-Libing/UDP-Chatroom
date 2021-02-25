#include "BaseData.h"
#include "Winsock2.h"
#include "stdio.h"

int RecvHead(SOCKET connSocket, DataHead *pDataHead)
{
	int nRet = ERROR_OTHER;
	int len = sizeof(SOCKADDR);
	//if (recv(connSocket, (char *)pDataHead, sizeof(DataHead), 0) < 0)
	if (recvfrom(connSocket, (char *)pDataHead, sizeof(DataHead), 0, (SOCKADDR*)&clientService, &len) == SOCKET_ERROR)
	{
		printf("RecvHead failed:%d \n", GetLastError());
		nRet = ERROR_RECV;
	}
	if (pDataHead->sign == DEFAULT_SIGN)
	{
		nRet = SUCCESS;
	}
	return nRet;
}

int SendHead(SOCKET connSocket, DataHead *pDataHead)
{
	int nRet = SUCCESS;
	int len = sizeof(SOCKADDR);
	//if (send(connSocket, (char*)pDataHead, sizeof(DataHead), 0) < 0)
	if (sendto(connSocket, (char*)pDataHead, sizeof(DataHead), 0, (SOCKADDR*)&clientService, len) == SOCKET_ERROR)
	{
		printf("SendHead failed:%d \n", GetLastError());
		nRet = ERROR_SEND;
	}
	return nRet;
}

int RecvData(SOCKET connSocket, char *buf, int bufLen)
{
	int nRet = SUCCESS;
	int len = sizeof(SOCKADDR);
	//if (recv(connSocket, buf, bufLen, 0) < 0)
	if (recvfrom(connSocket, buf, bufLen, 0, (SOCKADDR*)&clientService, &len) == SOCKET_ERROR)
	{
		printf("RecvData failed!\n");
		nRet = ERROR_RECV;
	}
	return nRet;
}

int SendData(SOCKET connSocket, char *buf, int bufLen)
{
	int nRet = SUCCESS;
	int len = sizeof(SOCKADDR);
	//if (send(connSocket, buf, bufLen, 0) < 0)
	if(sendto(connSocket, buf, bufLen, 0, (SOCKADDR*)&clientService, len) == SOCKET_ERROR)
	{
		printf("SendData failed!\n");
		nRet = ERROR_SEND;
	}
	return nRet;
}
