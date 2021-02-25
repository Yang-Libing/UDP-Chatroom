#include <map>
#include "Config.h"
using namespace std;

extern std::map<sockaddr_in_t, bool> curAddrIfLogin;

// ÓÃ»§Ä£¿é
bool IsNameRepeat(char *pUserName);

int Register(SOCKET srvSocket, sockaddr_in& currAddr);

int Login(SOCKET srvSocket, PUserOnlineNode& pCurrUserNode, bool *isLogin, sockaddr_in& currAddr);

int Logout(SOCKET *srvSocket, PUserOnlineNode pUserOnlienNode, bool *isLogin, sockaddr_in& currAddr);

int SetUserName(SOCKET *srvSocket, PUserOnlineNode pUserOnlienNode);

int SetUserPwd(SOCKET srvSocket, PUserOnlineNode pUserOnlienNode);
