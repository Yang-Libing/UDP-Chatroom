#include "ServerUserManage.h"
#include <stdio.h>

bool IsNameRepeat(char *pUserName)
{
	bool isRepeat = false;
	PUserDataNode userNode = g_pUserDataBegin;
	while (userNode)
	{
		if (strcmp(userNode->userData->userName, pUserName) == 0)
		{
			isRepeat = true;
			break;
		}
		userNode = userNode->pNext;
	}

	return isRepeat;
}

int Register(SOCKET srvSocket, sockaddr_in& currAddr)
{
	int nRet = SUCCESS;
	PUserData userData = (PUserData)malloc(sizeof(UserData));
	PUserDataNode userNode = (PUserDataNode)malloc(sizeof(UserDataNode));
	DataHead res;
	res.sign = DEFAULT_SIGN;
	res.cmd = CMD_REGIEST;
	res.dataLen = 0;
	if (userData == NULL || userNode == NULL)
	{
		printf("Malloc failed!\n");
		return ERROR_MALLOC;
	}

	nRet = RecvData(srvSocket, (char *)userData, sizeof(UserData), currAddr);
	if (nRet == SUCCESS)
	{
		// ����û����Ƿ��ظ�
		if (IsNameRepeat(userData->userName))
		{
			res.response = RES_FAULT;
			nRet = ERROR_OTHER;
			if (userData != NULL) {
				free(userData);
				userData = NULL;
			}
			if (userNode != NULL) {
				free(userNode);
				userNode = NULL;
			}
		}
		else
		{
			res.response = RES_SUCCESS;
			// ������û�����Ϣ���ڴ���
			userNode->userData = userData;
			userNode->pNext = NULL;
			g_pUserDataEnd->pNext = userNode;
			g_pUserDataEnd = userNode;
		}
		SendHead(srvSocket, &res, currAddr);
	}
	printf("%s ע��Ϊ�������û�\n", userData->userName);
	SaveConfig();
	return nRet;
}

int Login(SOCKET srvSocket, PUserOnlineNode& pCurrUserNode, bool *isLogin, sockaddr_in& currAddr)
{
	int nRet = ERROR_OTHER;
	bool isLoginSuccess = false;
	bool isLoginRepeat = false;
	PUserDataNode userNode = g_pUserDataBegin;
	PUserOnlineNode pOnlineNode = g_pUserOnlineBegin;
	DataHead res;
	PUserData userData = (PUserData)malloc(sizeof(UserData));
	if (userData == NULL)
	{
		printf("Malloc failed!\n");
		return ERROR_MALLOC;
	}

	res.sign = DEFAULT_SIGN;
	res.cmd = CMD_LOGIN;
	res.dataLen = 0;
	res.response = RES_FAULT;

	nRet = RecvData(srvSocket, (char *)userData, sizeof(UserData), currAddr);
	if (nRet == SUCCESS)
	{
		// �鿴�û����������Ƿ���ȷ
		while (userNode)
		{
			if (strcmp(userNode->userData->userName, userData->userName) == 0)
			{
				if (strcmp(userNode->userData->userPwd, userData->userPwd) == 0)
				{
					isLoginSuccess = true;
					break;
				}
			}
			userNode = userNode->pNext;
		}
		// ��½��Ϣ��ȷ
		if (isLoginSuccess)
		{
			// �����Ƿ��ظ���½
			while (pOnlineNode)
			{
				if (strcmp(userData->userName, pOnlineNode->userData->userName) == 0)
				{
					isLoginRepeat = true;
					break;
				}
				pOnlineNode = pOnlineNode->pNext;
			}

			// û���ظ���½
			if (!isLoginRepeat)
			{
				pCurrUserNode = (PUserOnlineNode)malloc(sizeof(UserOnlineNode));
				if (pCurrUserNode == NULL)
				{
					printf("pCurrUserNode malloc failed!\n");
					return ERROR_MALLOC;
				}

				res.response = RES_SUCCESS;
				*isLogin = true;
				// ����û���Ϣ�������û�������֮��
				pCurrUserNode->userData = userNode->userData;
				pCurrUserNode->userAddr = currAddr;
				pCurrUserNode->pNext = NULL;
				pCurrUserNode->pPrior = g_pUserOnlineEnd;

				sockaddr_in_t tmp;
				tmp.s = currAddr;
				curAddrIfLogin[tmp] = true;

				if (g_pUserOnlineBegin == NULL)
				{
					g_pUserOnlineBegin = pCurrUserNode;
				}

				if (g_pUserOnlineEnd != NULL)
				{
					g_pUserOnlineEnd->pNext = pCurrUserNode;
				}
				g_pUserOnlineEnd = pCurrUserNode;

				printf("%s ��½������\n", userData->userName);
			}
		}
	}

	SendHead(srvSocket, &res, currAddr);
	if (userData != NULL) {
		free(userData);
		userData = NULL;
	}
	return nRet;
}

int Logout(SOCKET *srvSocket, PUserOnlineNode& pUserOnlienNode, bool *isLogin)
{
	int nRet = SUCCESS;
	DataHead resHead;
	PUserOnlineNode pOnlineNode = g_pUserOnlineBegin;
	bool isExist = false;

	resHead.sign = DEFAULT_SIGN;
	resHead.cmd = CMD_LOGOUT;
	resHead.response = RES_FAULT;
	resHead.dataLen = 0;

	// ���Ҹ��û��Ƿ��������û��б�֮��
	while (pOnlineNode)
	{
		if (pUserOnlienNode == pOnlineNode)
		{
			isExist = true;
			break;
		}
		pOnlineNode = pOnlineNode->pNext;
	}

	if (isExist)
	{
		// �����û��ӵ�ǰ�����û����Ƴ�
		if (g_pUserOnlineBegin == pUserOnlienNode && g_pUserOnlineEnd == pUserOnlienNode) //��ֻ��һλ�û�ʱ
		{
			g_pUserOnlineBegin = NULL;
			g_pUserOnlineEnd = NULL;
		}
		else if (g_pUserOnlineBegin == pUserOnlienNode) //�����û�Ϊ��һ���û�ʱ
		{
			g_pUserOnlineBegin = pUserOnlienNode->pNext;
		}
		else if (g_pUserOnlineEnd == pUserOnlienNode) // �����û�Ϊ���һ���û�ʱ
		{
			g_pUserOnlineEnd = pUserOnlienNode->pPrior;
			g_pUserOnlineEnd->pNext = NULL;
		}
		else
		{
			pUserOnlienNode->pPrior->pNext = pUserOnlienNode->pNext;
			pUserOnlienNode->pNext->pPrior = pUserOnlienNode->pPrior;
		}
		resHead.response = RES_SUCCESS;
		*isLogin = false;

		sockaddr_in_t tmp;
		tmp.s = pUserOnlienNode->userAddr;
		curAddrIfLogin[tmp] = false;
		printf("%s �˳���������\n", pUserOnlienNode->userData->userName);
	}

	nRet = SendHead(*srvSocket, &resHead, pUserOnlienNode->userAddr);
	return nRet;
}

int SetUserName(SOCKET *srvSocket, PUserOnlineNode pUserOnlienNode)
{
	int nRet = SUCCESS;
	char newName[USER_NAME_LEN];
	DataHead resHead;
	resHead.sign = DEFAULT_SIGN;
	resHead.cmd = CMD_SET_USER_NAME;
	resHead.dataLen = 0;

	nRet = RecvData(*srvSocket, newName, USER_NAME_LEN, pUserOnlienNode->userAddr);
	if (nRet == SUCCESS)
	{
		// ��ѯ���û����Ƿ�����
		bool isRepeat = IsNameRepeat(newName);
		if (!isRepeat)
		{
			resHead.response = RES_SUCCESS;
			printf("%s �޸��û���Ϊ�� %s\n", pUserOnlienNode->userData->userName, newName);
			strcpy_s(pUserOnlienNode->userData->userName, USER_NAME_LEN, newName);
			SaveConfig();
		}
		else
		{
			resHead.response = RES_FAULT;
		}
	}
	else
	{
		resHead.response = RES_FAULT;
	}

	nRet = SendHead(*srvSocket, &resHead, pUserOnlienNode->userAddr);
	return nRet;
}

int SetUserPwd(SOCKET srvSocket, PUserOnlineNode pUserOnlienNode)
{
	int nRet = SUCCESS;
	char newPwd[USER_PWD_LEN];
	DataHead resHead;
	resHead.sign = DEFAULT_SIGN;
	resHead.cmd = CMD_SET_USER_PWD;
	resHead.dataLen = 0;

	nRet = RecvData(srvSocket, newPwd, USER_NAME_LEN, pUserOnlienNode->userAddr);
	if (nRet == SUCCESS)
	{
		// ��ѯ���û����Ƿ�����
		resHead.response = RES_SUCCESS;
		printf("%s �޸����û���\n", pUserOnlienNode->userData->userName);
		strcpy_s(pUserOnlienNode->userData->userPwd, USER_NAME_LEN, newPwd);
		SaveConfig();
	}
	else
	{
		resHead.response = RES_FAULT;
	}

	nRet = SendHead(srvSocket, &resHead, pUserOnlienNode->userAddr);
	return nRet;
}

