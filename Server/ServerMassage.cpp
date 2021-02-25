#include "ServerMassage.h"
#include "stdio.h"


int GroupMassage(SOCKET srvSocket, PUserOnlineNode pCurrUser, DataHead *head)
{
	int nRet = SUCCESS;
	PUserOnlineNode pOnlineNode = g_pUserOnlineBegin;
	char *buf = (char *)malloc(head->dataLen);
	if (buf == NULL)
	{
		printf("Malloc Failed!\n");
		return ERROR_MALLOC;
	}

	nRet = RecvData(srvSocket, buf, head->dataLen, pCurrUser->userAddr);
	if (nRet == SUCCESS)
	{
		while (pOnlineNode)
		{
			nRet = SendHead(srvSocket, head, pOnlineNode->userAddr);
			if (nRet == SUCCESS)
			{
				nRet = SendData(srvSocket, buf, head->dataLen, pOnlineNode->userAddr);
			}
			pOnlineNode = pOnlineNode->pNext;
		}
	}
	printf("%s 发送了群消息！\n", pCurrUser->userData->userName);
	return nRet;

}

int PrivateMassage(SOCKET srvSocket, PUserOnlineNode pCurrUser, DataHead *head)
{
	int nRet = SUCCESS;
	DataHead resHead;
	PrivateChat *pChat;
	PUserOnlineNode pOnlineNode = g_pUserOnlineBegin;
	char *buf = (char *)malloc(head->dataLen);
	char *toUser;

	if (buf == NULL)
	{
		printf("Malloc Failed!\n");
		return ERROR_MALLOC;
	}

	resHead.sign = DEFAULT_SIGN;
	resHead.cmd = CMD_PRIVATECHAT;
	resHead.dataLen = 0;

	nRet = RecvData(srvSocket, buf, head->dataLen, pCurrUser->userAddr);
	pChat = (PrivateChat *)buf;
	toUser = buf + sizeof(PrivateChat) + pChat->fromUserLen;

	while (pOnlineNode)
	{
		if (strcmp(toUser, pOnlineNode->userData->userName) == 0)
		{
			nRet = SendHead(srvSocket, head, pOnlineNode->userAddr);
			if (nRet != SUCCESS)
			{
				break;
			}
			// 发送消息
			nRet = SendData(srvSocket, buf, head->dataLen, pOnlineNode->userAddr);
			printf("[%s] 对 [%s] 发送了消息\n", pCurrUser->userData->userName, toUser);
			resHead.response = RES_SUCCESS;
			break;
		}
		pOnlineNode = pOnlineNode->pNext;
	}

	if (nRet != SUCCESS)
	{
		printf("[%s] 发生私聊信息失败！\n", pCurrUser->userData->userName);
		resHead.response = RES_FAULT;
	}

	nRet = SendHead(srvSocket, &resHead, pCurrUser->userAddr);
	return nRet;
}
