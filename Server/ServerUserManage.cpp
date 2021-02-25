#include "ServerUserManage.h"
#include "stdio.h"

bool IsNameRepeat(char *pUserName)
{
	//printf("In IsNameRepeat.\n");
	bool isRepeat = false;
	PUserDataNode userNode = g_pUserDataBegin;
	while (userNode)
	{
		//printf("In IsNameRepeat----1.\n");
		if (userNode) printf("userNode->userData->userName is: %s\n", userNode->userData->userName);
		else printf("userNode is NULL. \n");

		if (strcmp(userNode->userData->userName, pUserName) == 0)
		{
			//printf("In IsNameRepeat----2.\n");
			isRepeat = true;
			break;
		}
		//printf("In IsNameRepeat----3.\n");
		userNode = userNode->pNext;
	}
	//printf("In IsNameRepeat----4.\n");
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

	//printf("Enter Register.\n");
	nRet = RecvData(srvSocket, (char *)userData, sizeof(UserData), currAddr);
	//printf("After Register.\n");
	if (nRet == SUCCESS)
	{
		//printf("After Register----1.\n");
		// 检查用户名是否重复
		if (IsNameRepeat(userData->userName))
		{
			//printf("After Register----2.\n");
			res.response = RES_FAULT;
			nRet = ERROR_OTHER;
			free(userData);
			free(userNode);
			userData = NULL;
			userNode = NULL;
		}
		else
		{
			//printf("After Register----3.\n");
			res.response = RES_SUCCESS;
			// 添加新用户的信息到内存中
			userNode->userData = userData;
			userNode->pNext = NULL;
			g_pUserDataEnd->pNext = userNode;
			g_pUserDataEnd = userNode;
		}
		//printf("After Register----4.\n");
		SendHead(srvSocket, &res, currAddr);
		//printf("After Register----5.\n");
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

	//printf("Login 1...\n");
	nRet = RecvData(srvSocket, (char *)userData, sizeof(UserData), currAddr);
	//printf("Login 2...\n");
	if (nRet == SUCCESS)
	{
		//printf("Login 5...\n");
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
		//printf("Login 6...\n");
		// 登陆信息正确
		if (isLoginSuccess)
		{
			//printf("Login 7...\n");
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

			//printf("Login 8...\n");
			// 没有重复登陆
			if (!isLoginRepeat)
			{
				pCurrUserNode = (PUserOnlineNode)malloc(sizeof(UserOnlineNode));
				if (pCurrUserNode == NULL)
				{
					printf("pCurrUserNode malloc failed!\n");
					return ERROR_MALLOC;
				}

				//printf("Login 9...\n");
				res.response = RES_SUCCESS;
				*isLogin = true;
				// 添加用户信息到在线用户的链表之中
				pCurrUserNode->userData = userNode->userData;
				//pCurrUserNode->userSocket = connSocket;
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

	//printf("Login 3...\n");
	SendHead(srvSocket, &res, currAddr);
	//printf("Login 4...\n");
	free(userData);
	return nRet;
}

int Logout(SOCKET *srvSocket, PUserOnlineNode pUserOnlienNode, bool *isLogin, sockaddr_in& currAddr)
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
		if (g_pUserOnlineBegin == pUserOnlienNode && g_pUserOnlineEnd == pUserOnlienNode) //当只要一位用户时
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
		tmp.s = currAddr;
		curAddrIfLogin[tmp] = false;
		printf("%s 退出了聊天室\n", pUserOnlienNode->userData->userName);
	}

	nRet = SendHead(*srvSocket, &resHead, currAddr);
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

	//nRet = RecvData(pUserOnlienNode->userSocket, newName, USER_NAME_LEN);
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

	//nRet = SendHead(pUserOnlienNode->userSocket, &resHead);
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

	//nRet = RecvData(pUserOnlienNode->userAddr, newPwd, USER_NAME_LEN);
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

	//nRet = SendHead(pUserOnlienNode->userAddr, &resHead);
	nRet = SendHead(srvSocket, &resHead, pUserOnlienNode->userAddr);
	return nRet;
}

