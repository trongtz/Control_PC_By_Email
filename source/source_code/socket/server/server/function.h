#pragma once
#include "initServer.h"
#include<opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

using namespace cv;
void signal_callback_handler(int signum, SOCKET clientSocket);
void showCur(bool CursorVisibility);
void openWebcam(SOCKET ClientSocket, int n);
void stopWebcam(SOCKET ClientSocket);

LPWSTR ConvertToLPWSTR(const char* str);
string ConvertLPWSTRToString(LPWSTR lpwstr);
void saveBitmap(HBITMAP hBitmap, const char* filename);
void takeScreenshot(const char* filename, SOCKET ClientSocket);
string ListInstalledApplications(SOCKET ClientSocket);
string getFileNameFromPath(string filePath);
void deleteFile(string filePath, SOCKET ClientSocket);
bool copyFile(const string& source, const string& destination, SOCKET ClientSocket);
void shutDownOrRestart(int choice, SOCKET ClientSocket);

struct Application {
    HWND hwnd;
    wstring windowTitle;
    DWORD processID;
};

struct Command {
    string act;
    string path1;
    string path2;
};

void listProcess(SOCKET ClientSocket);
void stopApp(vector<Application>& apps, int choice, SOCKET ClientSocket);
string ConvertWStringToString(const wstring& wstr);
string printListOfProcess(const vector<Application>& app);
void terminateProcessByID(DWORD processID);
void closeApplicationByHWND(HWND hwnd, SOCKET ClientSocket);
vector<Application> listApplications();
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void keyLogger(int second, SOCKET ClientSocket);
void checkBatteryStatus(SOCKET ClientSocket); 
void lockKeyboardAndMouse(int seconds, SOCKET ClientSocket);
void setWallpaper(string imagePath, SOCKET ClientSocket);
void openApp(string name, SOCKET ClientSocket);
void checkStorage(SOCKET ClientSocket);


bool isLink(string inp);
string lowerCase(string inp);
bool isNum(string inp);
Command getCommandInfoFromEmail(string inp);
