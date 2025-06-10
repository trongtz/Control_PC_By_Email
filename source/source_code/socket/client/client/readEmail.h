#pragma once
#include <iostream>
#include <string>
#include <curl/curl.h>
#include<fstream>
using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* userData);
int countEmailsInGmail(string username, string password);
string getEmailInfo(const string& rawEmail);
string readEmail(const string & username, const string & appPassword, int index);