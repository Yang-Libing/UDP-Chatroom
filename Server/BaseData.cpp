#include "BaseData.h"
#include "Winsock2.h"
#include "stdio.h"

int RecvHead(SOCKET srvSocket, DataHead *pDataHead, sockaddr_in& newLoginer)
{
	int nRet = ERROR_OTHER;
	int len = sizeof(SOCKADDR);
	if (recvfrom(srvSocket, (char *)pDataHead, sizeof(DataHead), 0, (SOCKADDR*)&newLoginer, &len) == SOCKET_ERROR)
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

int SendHead(SOCKET srvSocket, DataHead *pDataHead, sockaddr_in& currAddr)
{
	int nRet = SUCCESS;
	int len = sizeof(SOCKADDR);
	if(sendto(srvSocket, (char*)pDataHead, sizeof(DataHead), 0, (SOCKADDR*)&currAddr, len) == SOCKET_ERROR)
	{
		printf("SendHead failed:%d \n", GetLastError());
		nRet = ERROR_SEND;
	}
	return nRet;
}

int RecvData(SOCKET srvSocket, char *buf, int bufLen, sockaddr_in& currAddr)
{
	int nRet = SUCCESS;
	int len = sizeof(SOCKADDR);

	if(recvfrom(srvSocket, buf, bufLen, 0, (SOCKADDR*)&currAddr, &len) == SOCKET_ERROR)
	{
		printf("RecvData failed:%d \n", GetLastError());
		nRet = ERROR_RECV;
	}
	return nRet;
}

int SendData(SOCKET srvSocket, char *buf, int bufLen, sockaddr_in& currAddr)
{
	int nRet = SUCCESS;
	int len = sizeof(SOCKADDR);

	if (sendto(srvSocket, buf, bufLen, 0, (SOCKADDR*)&currAddr, len) == SOCKET_ERROR)
	{
		printf("SendData failed!\n");
		nRet = ERROR_SEND;
	}
	return nRet;
}
