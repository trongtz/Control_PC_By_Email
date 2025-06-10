#include "initClient.h"
#include "readEmail.h"

SOCKET clientSocket;
sockaddr_in addr;

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(nullptr);

    bool flag = checkDLL();
    if (flag == false) return -1;
    else {
        ifstream inp("ip_address.txt", ios::in);
        string ip = "";
        getline(inp, ip);
        inp.close();

        clientSocket = createSocket();
        addr = setAddress(ip);

        // cap nhat lai so luong mail va vi tri mail chuan bi duoc thuc thi(alreadyProcess)
        int mailInGmail = countEmailsInGmail("thitkhomamruot7749@gmail.com", "iiqi bkuv pzxw uvqk") - 1;
        ofstream ifile("data.txt", ios::out);
        if (ifile.is_open()) {
            ifile << mailInGmail;
        }
        ifile.close();
       
        cout << "Connecting to server....." << endl;
        while (true) {
            if (createConnection(clientSocket, addr)) break;
        }

        // Doc noi dung email va tien hanh gui toi server 
        bool connect = true;
        while (connect==true) {
            int alreadyProcess = countEmailsInGmail("thitkhomamruot7749@gmail.com", "iiqi bkuv pzxw uvqk") - 1;
            int lastProcess = 0;
            ifstream ifile("data.txt", ios::in);
            if (ifile.is_open()) {
                string line = "";
                getline(ifile, line);
                lastProcess = stoi(line);
            }
            ifile.close();
            while (alreadyProcess > lastProcess) {
                lastProcess++;
                string message = readEmail("thitkhomamruot7749@gmail.com", "iiqi bkuv pzxw uvqk", lastProcess);
                string MailAdress = message.substr(0, message.find('-'));
                message.erase(0, message.find('-') + 1);

                if (message == "add thitkhomamruot7749")
                {
                    ifstream ifile("mailGrantedPermission.txt", ios::in);
                    if (ifile.is_open())
                    {
                        string line = "";
                        bool check = false;
                        while (getline(ifile, line))
                        {
                            if (line == MailAdress)
                            {
                                check = true;
                                break;
                            }
                        }
                        if (check == true)
                        {
                            responseToEmail("Mail da ton tai trong danh sach cap quyen truy cap.", MailAdress, NULL);                            
                        }
                        else
                        {
                            ofstream ofile("mailGrantedPermission.txt", ios::app);
                            if (ofile.is_open())
                            {
                                ofile << MailAdress << endl;
                            }
                            ofile.close();
                            responseToEmail("Da them mail vao danh sach cap quyen truy cap.", MailAdress, NULL);                           
                        }
                    }
                    ifile.close();
                    continue;
                }
                else
                {
                    bool legit = false;
                    ifstream authorizedUsers("mailGrantedPermission.txt", ios::in);
                    if (authorizedUsers.is_open())
                    {
                        string line = "";
                        while (!authorizedUsers.eof()) {
                            getline(authorizedUsers, line);
                            if (MailAdress == line) {
                                legit = true;
                                break;
                            }
                        }
                    }
                    if (legit == true)
                    {
                        if (message != "") {
                            int sendResult = send(clientSocket, message.c_str(), message.length(), 0);
                            if (sendResult == SOCKET_ERROR) {
                                cerr << "Failed to send command to server\n";
                            }
                            else {
                                cout << "Command was sent to server successfully\n";
                                message = "";
                            }
                        }
                        receiveFromServer(clientSocket, MailAdress, connect);

                    }
                    else continue;
                }
                
            }
            ofstream ofile("data.txt", ios::out);
            ofile << alreadyProcess;
            ofile.close();
        }
        // Dong socket va don dep
        closesocket(clientSocket);
        WSACleanup();
        cout << "Client da dong.";
    }
    return 0;
}