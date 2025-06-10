#include "initServer.h"
#include "function.h"


bool checkDLL() {
    WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaerr < 0) return false;
    return true;
}

SOCKET createSocket() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return sock;
}

sockaddr_in setAddress() {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.S_un.S_addr = INADDR_ANY; // Lắng nghe trên tất cả các địa chỉ3 IP
    address.sin_port = htons(8080);
    return address;
}

void createConnection(SOCKET& sock, sockaddr_in& address) {
    if (connect(sock, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        cout << "                    ";
        cout << "No connection established" << "\n";
        return;
    }
    else {
        cout << "                    ";
        cout << "Connection initialized successfully" << "\n";
    }
}

void bindSocket(SOCKET& sock, sockaddr_in& address) {
    if (::bind(sock, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        cout << "                    ";
        cout << "Bind Failed" << "\n";
        return;
    }
    else {
        cout << "                    ";
        cout << "Bind Successfully" << "\n";
    }
}

void setListen(SOCKET& sock, int n) {
    if (listen(sock, n) == SOCKET_ERROR) {
        cout << "                    ";
        cout << "Socket not in Listen State" << "\n";
    }
    else {
        cout << "                    ";
        cout << "Socket in Listen State" << "\n";
    }
}

void acptConnection(SOCKET& clientSocket, SOCKET& serverSocket, bool& connect) {
    clientSocket = accept(serverSocket, NULL, NULL); // Chấp nhận kết nối mới
    if (clientSocket == INVALID_SOCKET) {
        cout << "                    ";
        cerr << "Error: accepting connection\n";
        return;
    }
    cout << "                    ";
    cout << "Connected successfully\n";
    cout << "________________________________________________________________________________________________________________________\n";
    receiveCommand(clientSocket, connect);
    closesocket(clientSocket);
}



// Nhan lenh va thuc hien lenh
void receiveCommand(SOCKET clientSocket,bool& connect)
{
    while (connect==true) {
        char buffer[1024];
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived == SOCKET_ERROR)
        {
            cout << "                    ";
            cerr << "Error: Failed to receive data from the client.\n";
            break;
        }
        else if (bytesReceived > 0)
        {
            buffer[bytesReceived] = '\0';
            cout << "                    ";
            cout << "From client: " << buffer << "\n";
            Command c = getCommandInfoFromEmail(string(buffer));
            if (c.act == "takefile") {
                const char* path1 = c.path1.c_str();
                responseToClient(clientSocket, "Take file successfully", path1);
            }
            else if (c.act == "recording") openWebcam(clientSocket,stoi(c.path1));
            else if (c.act == "stopwebcam") stopWebcam(clientSocket);
            else if (c.act == "screenshot") {
                takeScreenshot("C:\\Users\\Public\\screenshot.bmp", clientSocket); // Chup anh man hinh
            }
            else if (c.act == "deletefile") {
                deleteFile(string(c.path1), clientSocket); // Xoa file
            }
            else if (c.act == "copyfile") {
                copyFile(string(c.path1), string(c.path2), clientSocket); // Sao chep file
            }
            else if (c.act == "listapp") ListInstalledApplications(clientSocket); // Liet ke cac ung dung da cai dat
            else if (c.act == "restart") {
                shutDownOrRestart(2, clientSocket); // Khoi dong lai may tinh
            }
            else if (c.act == "shutdown") {
                shutDownOrRestart(1, clientSocket); // Tat may tinh
            }
            else if (c.act == "listprocess") listProcess(clientSocket); // Liet ke va dung cac ung dung
            else if (c.act == "stopprocess") {
                vector<Application> apps = listApplications();
                stopApp(apps, stoi(c.path1), clientSocket); // Dung ung dung
            }
            else if (c.act == "keylogger") {
                if (isNum(c.path1) && c.path1 != "") keyLogger(stoi(c.path1), clientSocket); // Ghi lai thao tac phim
                else responseToClient(clientSocket, "Please enter a specific time to keylog!", NULL);
            }
            else if (c.act == "checkbattery") checkBatteryStatus(clientSocket); // Kiem tra pin
            else if (c.act == "lockkeyboard") {
                if (isNum(c.path1) && c.path1 != "") lockKeyboardAndMouse(stoi(c.path1), clientSocket);// Khoa ban phim
                else responseToClient(clientSocket, "Please enter a specific time to lock keyboard!", NULL);
            }
            else if (c.act == "changebackground") setWallpaper(c.path1, clientSocket);
            else if (c.act == "open") openApp(c.path1, clientSocket);
            else if (c.act == "checkstorage") checkStorage(clientSocket);
            else if (c.act == "stopaccess") {
                responseToClient(clientSocket, "Stop connection successfully!", NULL);
                connect = false;
                return;
            }
            else {
                string respondText = "Command not valid: " + string(buffer) + ". Please try again!";
                responseToClient(clientSocket, respondText, NULL);
            }
            cout << "________________________________________________________________________________________________________________________\n";
        }
    }
}
