#pragma once
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <signal.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include "lib.h"
#include <windows.h>  

using namespace EASendMailObjLib;
using namespace std;
using namespace this_thread;

#pragma warning(disable: 4996)
#pragma comment(lib, "ws2_32.lib")
#define PORT 8080
#define BUFFER_SIZE 4096
#define MAX 100

//SERVER
bool checkDLL();
SOCKET createSocket();
sockaddr_in setAddress();
void createConnection(SOCKET& sock, sockaddr_in& address);
void bindSocket(SOCKET& sock, sockaddr_in& address);
void setListen(SOCKET& sock,int n);
void acptConnection(SOCKET& clientSocket, SOCKET& serverSocket, bool& connect);

void receiveCommand(SOCKET clientSocket,bool& connect);
void responseToClient(SOCKET ClientSocket, string response, const char* path);
void SendFileToClient(SOCKET clientSocket, string file_path);
void gotoxy(int x, int y);
void decorate();
