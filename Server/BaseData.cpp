#include "BaseData.h"
#include "Winsock2.h"
#include "stdio.h"

int RecvHead(SOCKET srvSocket, DataHead *pDataHead, sockaddr_in& newLoginer)
{
	int nRet = ERROR_OTHER;
	//sockaddr_in currAddr;
	int len = sizeof(SOCKADDR);
	//if (recv(connSocket, (char *)pDataHead, sizeof(DataHead), 0) < 0)
	if (recvfrom(srvSocket, (char *)pDataHead, sizeof(DataHead), 0, (SOCKADDR*)&newLoginer, &len) == SOCKET_ERROR)
	{
		printf("RecvHead failed:%d \n", GetLastError());
		nRet = ERROR_RECV;
	}
	/*if (pDataHead->cmd == CMD_LOGIN) {
		isNewLoginer = true;
	}*/
	/*if (pDataHead->cmd == CMD_REGIEST) {
		printf("newLoginer: %s\n", inet_ntoa(newLoginer.sin_addr));
	}*/
	if (pDataHead->sign == DEFAULT_SIGN)
	{
		nRet = SUCCESS;
	}
	printf("Enter RecvHead..........\n");
	return nRet;
}

int SendHead(SOCKET srvSocket, DataHead *pDataHead, sockaddr_in& currAddr)
{
	int nRet = SUCCESS;
	int len = sizeof(SOCKADDR);
	//if (send(connSocket, (char*)pDataHead, sizeof(DataHead), 0) < 0)
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
	//printf("Enter RecvData,and currAddr is: %s\n", inet_ntoa(currAddr.sin_addr));
	//if (recv(connSocket, buf, bufLen, 0) < 0)
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
	//if (send(connSocket, buf, bufLen, 0) < 0)
	if (sendto(srvSocket, buf, bufLen, 0, (SOCKADDR*)&currAddr, len) == SOCKET_ERROR)
	{
		printf("SendData failed!\n");
		nRet = ERROR_SEND;
	}
	return nRet;
}
