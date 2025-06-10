#include "initClient.h"
#include "display.h"

// Neu khong thanh cong, tra ve false
bool checkDLL() {
    WSADATA wsaData;
    int wsaerr = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaerr < 0) return false;
    return true;
}

// Khoi tao socket
SOCKET createSocket() {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    return sock;
}

// Tao cau truc address cho dia chi server
sockaddr_in setAddress(string ip) {
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
    address.sin_port = htons(8080); 
    return address;
}

// Ket noi socket voi server
int createConnection(SOCKET& sock, sockaddr_in& address) {
    if (connect(sock, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        return 0;
    }
    else cout << "Connected Successfully" << "\n";
    return 1;
}

// phan hoi email ket qua thuc hien o server da chuyen ve client
void responseToEmail(string response,string userMail, const char* path) {
    const int ConnectNormal = 0;
    const int ConnectSSLAuto = 1;
    const int ConnectSTARTTLS = 2;
    const int ConnectDirectSSL = 3;
    const int ConnectTryTLS = 4;
    ::CoInitialize(NULL); // Khoi tao COM

    IMailPtr oSmtp = NULL;
    oSmtp.CreateInstance(__uuidof(EASendMailObjLib::Mail)); // Tao doi tuong email
    oSmtp->LicenseCode = _T("TryIt"); // Ma giay phep (demo)

    // Thiet lap thong tin gui mail
    oSmtp->FromAddr = _T("thitkhomamruot7749@gmail.com");
    oSmtp->AddRecipientEx(userMail.c_str(), 0);
    oSmtp->Subject = _T("RESPONSE FROM CONTROLLED PC");

    BSTR bstrResponse = SysAllocString(wstring(response.begin(), response.end()).c_str());
    oSmtp->BodyFormat = 1;
    oSmtp->BodyText = bstrResponse;
    SysFreeString(bstrResponse); // Giai phong bo nho BSTR

    // Thiet lap thong tin SMTP
    oSmtp->ServerAddr = _T("smtp.gmail.com");
    oSmtp->UserName = _T("thitkhomamruot7749@gmail.com");
    oSmtp->Password = _T("iiqi bkuv pzxw uvqk");
    oSmtp->ServerPort = 587;
    oSmtp->ConnectType = ConnectSTARTTLS;
    
    // Neu co file ket qua dinh kem, them vao de gui ve email
    if (path != NULL) {
        TCHAR tPath[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, path, -1, tPath, MAX_PATH);
        oSmtp->AddAttachment(tPath);
    }
    _tprintf(_T("\nStart to send email ...\r\n"));

    // Gui email
    if (oSmtp->SendMail() == 0) {
        _tprintf(_T("Email was sent successfully!\r\n"));
    }
    else {
        _tprintf(_T("Failed to send email with the following error: %s\r\n"),
            (const TCHAR*)oSmtp->GetLastErrDescription());
    }
    cout << "gui thanh cong" << endl; // Thong bao gui email thanh cong
}

// ham chuan hoa phan duoi cua file tu server gui ve
void trim(string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) {
        str = "";
    }
    else {
        size_t last = str.find_last_not_of(" \t\n\r");
        str = str.substr(first, (last - first + 1));
    }
}

// nhan file/message tu server gui tra ve sau khi thuc hien
void receiveFromServer(SOCKET ClientSocket, string userEmail,bool& connect) {
    char buffer[BUFFER_SIZE];

    // nhan header
    int bytesReceived = recv(ClientSocket, buffer, BUFFER_SIZE, 0);
    if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
        cerr << "Failed to receive header or connection closed\n";
        return;
    }
    buffer[bytesReceived] = '\0';
    string header(buffer);
    cout << "Received header: " << header << endl;

    string dataType;
    size_t contentLength = 0;

    size_t typePos = header.find("TYPE:");
    if (typePos != string::npos) {
        size_t endPos = header.find("\n", typePos);
        dataType = header.substr(typePos + 5, endPos - (typePos + 5));
        trim(dataType);
    }

    if (dataType == "text") {
        // Phan biet du lieu nhan vao la mot thong bao kieu string
            bytesReceived = recv(ClientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesReceived == SOCKET_ERROR) {
                responseToEmail("Failed to receive response\n",userEmail, NULL);
                return;
            }
            buffer[bytesReceived] = '\0';

            ////////////////////////////
            string response(buffer);
            if (response == "Stop connection successfully!") {
                response = customMailDisplay(response);
                ///////////////////////////
                responseToEmail(response, userEmail, NULL);
                connect = false;
                return;
            }
            response = customMailDisplay(response);
            ///////////////////////////
            responseToEmail(response, userEmail, NULL);
    }
    else if (dataType == "file") {
        // Phan biet du lieu nhan vao la 1 file
        string fileName = "received_file";
        string fileType;
        size_t fileTypePos = header.find("FILE_TYPE:");
        if (fileTypePos != string::npos) {
            size_t endPos = header.find("\n", fileTypePos);
            fileType = header.substr(fileTypePos + 10, endPos - (fileTypePos + 10));
            trim(fileType);
        }

        string fileExtension = fileType.empty() ? ".bin" : "." + fileType;
        string filePath = "C:\\Users\\Public\\" + fileName + fileExtension;

        ofstream outFile(filePath, ios::binary);
        if (!outFile.is_open()) {
            cerr << "Failed to open file for writing\n";
            return;
        }

        // nhan size cua file
        int file_size = 0;
        int size_bytes_received = recv(ClientSocket, reinterpret_cast<char*>(&file_size), sizeof(file_size), 0);
        if (size_bytes_received == SOCKET_ERROR) {
            cerr << "Failed to receive file size!" << endl;
            return;
        }

        // nhan data cua file
        size_t receivedBytes = 0;
        while (receivedBytes < file_size) {
            bytesReceived = recv(ClientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesReceived == SOCKET_ERROR) {
                cerr << "Failed to receive file data\n";
                outFile.close();
                return;
            }
            outFile.write(buffer, bytesReceived);
            receivedBytes += bytesReceived;
        }
        outFile.close();
        cout << "File received and saved to: " << filePath << endl;

        /////////////////////////////
        //int bytesRec = recv(ClientSocket, buffer, BUFFER_SIZE, 0);
        //if (bytesRec == SOCKET_ERROR || bytesRec == 0) {
        //    cerr << "Failed to receive response or connection closed\n";
        //    return;
        //}
        ////buffer[bytesRec] = '\0';
        //string response(buffer);
        //cout << "Received response: " << response << endl;

        string response = "";
        if (fileType == "txt") {
            string line;
            ifstream file(filePath, ios::in);
            getline(file, line);
            if (line == "LIST OF INSTALLED APPLICATIONS:") {
                response = response + line + "\n";
                while (!file.eof()) {
                    getline(file, line);
                    if (line != "") response = response + line + "\n";
                }
                file.close();
                response = customMailDisplay(response);
                responseToEmail(response, userEmail, NULL);
            }
            else {
                file.close();
                response = customMailDisplay(response);
                responseToEmail(response, userEmail, filePath.c_str());
            }
        }
        else {
            //////////////////////////////////////
            //string response = "";
            response = customMailDisplay(response);
            ///////////////////////////////////////
            responseToEmail(response, userEmail, filePath.c_str());
        }
    }
    else {
        cerr << "Unknown data type\n";
    }
}