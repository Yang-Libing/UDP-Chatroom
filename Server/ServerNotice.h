#include "Config.h"

// ����ģ��
int SendOnlineUserList(SOCKET srvSocket, PUserOnlineNode pCurrUser);

int SendAllUserList(SOCKET srvSocket, PUserOnlineNode pCurrUser);

int SendCurrentUser(SOCKET srvSocket, PUserOnlineNode pCurrUser);