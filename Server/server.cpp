#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN
#endif #include <windows.h> 

#include <WS2tcpip.h>
#include <IPHlpApi.h>
#include <stdio.h> 
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <vector>
#include <map>
#include "Config.h"
#include "ServerUserManage.h"
#include "ServerMassage.h"
#include "ServerNotice.h"
#include "ServerAdminManage.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define MAX_THREAD_COUNTS 20 

PUserDataNode g_pUserDataBegin;
PUserDataNode g_pUserDataEnd;

PUserOnlineNode g_pUserOnlineBegin;
PUserOnlineNode g_pUserOnlineEnd;

map<sockaddr_in_t, bool> curAddrIfLogin;

HANDLE g_hMutex;
bool g_isExit = false;

// 初始化监听套接字
int InitListenSocket(SOCKET *pListenSocket)
{
	int nRet = 0;
	int iResult;
	sockaddr_in service;

	service.sin_family = AF_INET;
	//service.sin_addr.s_addr = inet_addr("127.0.0.1");
	//service.sin_addr.s_addr = ADDR_ANY;
	service.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	service.sin_port = htons(DEFAULT_PORT);
	//*pListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	*pListenSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (*pListenSocket != INVALID_SOCKET)
	{
		iResult = bind(*pListenSocket, (SOCKADDR*)&service, sizeof(SOCKADDR));
		if (iResult == SOCKET_ERROR)
		{
			printf("bind failed with error: %d\n", WSAGetLastError());
			nRet = 1;
		}
		else {
			printf("bind success.\n");
		}
	}
	else
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
	}
	return nRet;
}

unsigned __stdcall ClientThread(void *pSocket)
{
	int nRet = 0;
	DataHead head;
	sockaddr_in currAddr;
	bool isLogin = false;
	int tmp_count = 0;
	//SOCKET clientSocket = INVALID_SOCKET;
	SOCKET *pListenSocket = (SOCKET*)pSocket;
	PUserOnlineNode currOnlineUser = NULL;
	int addrLen = sizeof(currAddr);
	/*currOnlineUser = (PUserOnlineNode)malloc(sizeof(UserOnlineNode));
	if (currOnlineUser == NULL)
	{
		printf("Malloc failed!\n");
		return ERROR_MALLOC;
	}*/

	while (!g_isExit)
	{
		// 原子锁
		/*WaitForSingleObject(g_hMutex, INFINITE);
		clientSocket = accept(*pListenSocket, (sockaddr *)&currAddr, &addrLen);
		ReleaseMutex(g_hMutex);*/
		if (*pListenSocket == INVALID_SOCKET)
		{
			printf("accept failed: %d\n", GetLastError());
			nRet = ERROR_ACCEPT;
			continue;
		}

		while (!g_isExit && nRet == SUCCESS)
		{
			/*
			目前存在一个严重的Bug:
			    多个用户登陆时，服务器崩溃。
			可以尝试：
			1. 查看客户端连接的端口号
			2. isLogin这个变量好像有问题，第一个用户登陆后是true，但对于第二个用户
			来说应该是false，但是此时还是true。
			3. 使用curAddrIfLogin[tmp]时，问题：登陆2个用户，不管是先登的还是后登的，
			先点了获取当前在线列表，都是后登的用户界面重复显示。
			4. currOnlineUser这个变量有问题，这个变量只malloc了一次，而每登陆一个用户应该再malloc一次!
			5. malloc重新调整了，但是还没进行free处理！
			6. 
			*/

			memset(&currAddr, 0, sizeof(currAddr));
			//nRet = RecvHead(clientSocket, &head);
			nRet = RecvHead(*pListenSocket, &head, currAddr);
			//printf("tmp_count=%d\n", ++tmp_count);
			if (nRet != SUCCESS)
			{
				break;
			}

			sockaddr_in_t tmp;
			tmp.s = currAddr;
			//printf("Enter t1...\n");
			//if (!isLogin) // 用户未登陆
			if(curAddrIfLogin[tmp] != true)  // 用户未登陆
			{
				//printf("Enter t2...\n");
				switch (head.cmd)
				{
				case CMD_REGIEST:
					nRet = Register(*pListenSocket, currAddr);  //clientSocket
					break;
				case CMD_LOGIN:
					//printf("Enter t22...\n");
					nRet = Login(*pListenSocket, currOnlineUser, &isLogin, currAddr);   //clientSocket
					break;
				default:
					nRet = ERROR_OTHER;
					break;
				}
			}
			else         // 用户已登陆
			{
				//printf("Enter t3...\n");

				//更新currOnlineUser的值
				currOnlineUser = g_pUserOnlineBegin;
				while (currOnlineUser)
				{
					if(currAddr.sin_port == currOnlineUser->userAddr.sin_port &&
						currAddr.sin_addr.s_addr == currOnlineUser->userAddr.sin_addr.s_addr)  break;
					currOnlineUser = currOnlineUser->pNext;
				}

				switch (head.cmd)
				{
				case CMD_LOGOUT:
					nRet = Logout(pListenSocket, currOnlineUser, &isLogin, currAddr); //&clientSocket
					break;
				case CMD_GROUPCHAT:
					nRet = GroupMassage(*pListenSocket, currOnlineUser, &head);
					break;
				case CMD_PRIVATECHAT:
					nRet = PrivateMassage(*pListenSocket, currOnlineUser, &head);
					break;
				case CMD_SET_USER_NAME:
					nRet = SetUserName(pListenSocket, currOnlineUser);
					break;
				case  CMD_SET_USER_PWD:
					nRet = SetUserPwd(*pListenSocket, currOnlineUser);
					break;
				case CMD_GET_ALL_USER:
					nRet = SendAllUserList(*pListenSocket, currOnlineUser);
					break;
				case CMD_GET_ONLINE_USER:
					nRet = SendOnlineUserList(*pListenSocket, currOnlineUser);
					break;
				case CMD_GET_CURRENT_USER:
					nRet = SendCurrentUser(*pListenSocket, currOnlineUser);
					break;
				case CMD_SET_USER_OFFLINE:
					nRet = SetUserOffLine(*pListenSocket, currOnlineUser);
					break;
				case CMD_SET_USER_LEVEL:
					nRet = SetUserLevel(*pListenSocket, currOnlineUser);
					break;
				default:
					break;
				}
			}
		}
		//printf("Enter t4...\n");
		// 发送异常，用户退出登陆
		if (nRet != SUCCESS)
		{
			Logout(pListenSocket, currOnlineUser, &isLogin, currAddr);  //&clientSocket
		}
		closesocket(*pListenSocket);   //clientSocket
	}

	free(currOnlineUser);
	currOnlineUser = NULL;
	return nRet;
}

int main()
{
	int iResult;
	WSADATA wsaData;
	SOCKET listeningSocket = INVALID_SOCKET;
	//HANDLE aThread[MAX_THREAD_COUNTS];
	HANDLE aThread;
	char ch;
	// 初始化winsock 
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}

	// 创建互斥锁
	g_hMutex = CreateMutex(NULL, false, NULL);
	if (g_hMutex == NULL)
	{
		printf("CreateMutex error: %d\n", GetLastError());
		return 1;
	}

	InitConfig();

	InitListenSocket(&listeningSocket);
	//iResult = listen(listeningSocket, SOMAXCONN);
	if (listeningSocket == SOCKET_ERROR)
	{
		printf("InitListenSocket failed with error: %d\n", WSAGetLastError());
	}
	else
	{
		// 接收缓冲区
		int nRecvBuf = 32 * 1024;//设置为32K
		setsockopt(listeningSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int));

		//发送缓冲区
		int nSendBuf = 32 * 1024;//设置为32K
		setsockopt(listeningSocket, SOL_SOCKET, SO_SNDBUF, (const char*)&nSendBuf, sizeof(int));

		/*for (int i = 0; i < MAX_THREAD_COUNTS; i++)
		{
			aThread[i] = (HANDLE)_beginthreadex(NULL, 0, &ClientThread, (void*)&listeningSocket, 0, NULL);
		}*/
		aThread = (HANDLE)_beginthreadex(NULL, 0, &ClientThread, (void*)&listeningSocket, 0, NULL);

		printf("服务启动成功，退出请输入（E）: \n");
		while ((ch = getchar()) != NULL)
		{
			ch = tolower(ch);
			if (ch = 'e')
			{
				g_isExit = true;
				break;
			}
		}

		WaitForMultipleObjects(MAX_THREAD_COUNTS, &aThread, true, 500);
		for (int i = 0; i < MAX_THREAD_COUNTS; i++)
		{
			CloseHandle(aThread);
		}
	}

	// 释放资源
	CloseConfig();
	CloseHandle(g_hMutex);
	closesocket(listeningSocket);
	WSACleanup();

	return 0;
}