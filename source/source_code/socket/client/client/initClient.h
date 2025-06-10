#include "lib.h"

using namespace EASendMailObjLib;
using namespace std;
using namespace this_thread;

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 4096

bool checkDLL();
SOCKET createSocket();
sockaddr_in setAddress(string ip);
int createConnection(SOCKET& sock, sockaddr_in& address);
void responseToEmail(string response, string userMail, const char* path);
void trim(string& str);
void receiveFromServer(SOCKET ClientSocket, string userEmail,bool& connect);
