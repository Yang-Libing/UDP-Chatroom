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
		// 检查用户名是否重复
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
			// 添加新用户的信息到内存中
			userNode->userData = userData;
			userNode->pNext = NULL;
			g_pUserDataEnd->pNext = userNode;
			g_pUserDataEnd = userNode;
		}
		SendHead(srvSocket, &res, currAddr);
	}
	printf("%s 注册为聊天室用户\n", userData->userName);
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
		// 查看用户名和密码是否正确
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
		// 登陆信息正确
		if (isLoginSuccess)
		{
			// 查找是否重复登陆
			while (pOnlineNode)
			{
				if (strcmp(userData->userName, pOnlineNode->userData->userName) == 0)
				{
					isLoginRepeat = true;
					break;
				}
				pOnlineNode = pOnlineNode->pNext;
			}

			// 没有重复登陆
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
				// 添加用户信息到在线用户的链表之中
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

				printf("%s 登陆聊天室\n", userData->userName);
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

	// 查找该用户是否在在线用户列表之中
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
		// 将该用户从当前在线用户中移除
		if (g_pUserOnlineBegin == pUserOnlienNode && g_pUserOnlineEnd == pUserOnlienNode) //当只有一位用户时
		{
			g_pUserOnlineBegin = NULL;
			g_pUserOnlineEnd = NULL;
		}
		else if (g_pUserOnlineBegin == pUserOnlienNode) //当该用户为第一个用户时
		{
			g_pUserOnlineBegin = pUserOnlienNode->pNext;
		}
		else if (g_pUserOnlineEnd == pUserOnlienNode) // 当该用户为最后一个用户时
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
		printf("%s 退出了聊天室\n", pUserOnlienNode->userData->userName);
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
		// 查询新用户名是否重名
		bool isRepeat = IsNameRepeat(newName);
		if (!isRepeat)
		{
			resHead.response = RES_SUCCESS;
			printf("%s 修改用户名为： %s\n", pUserOnlienNode->userData->userName, newName);
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
		// 查询新用户名是否重名
		resHead.response = RES_SUCCESS;
		printf("%s 修改了用户名\n", pUserOnlienNode->userData->userName);
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

