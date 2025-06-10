#include "initServer.h"
#include "function.h"

SOCKET serverSocket;
sockaddr_in address;

void signal_callback_handler(int signum) {
    closesocket(serverSocket);
    WSACleanup();
    cout << "                    ";
    cout << "Server closed.\n";
    exit(signum);
}

int main()
{
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    showCur(0);

    bool flag = checkDLL();
    if (flag == false) {
        cout << "                    ";
        cout << "Startup failed.\n";
        return -1;
    }
    else {

        decorate();
        serverSocket = createSocket();
        SOCKET clientSocket = createSocket();
        address = setAddress();

        bindSocket(serverSocket, address);
        setListen(serverSocket, 1); //so client toi da duoc ket noi vao server
        signal(SIGINT, signal_callback_handler);
        bool connect = true;
        while (connect==true) {           
            acptConnection(clientSocket, serverSocket, connect);
        }
 
        closesocket(serverSocket);
        WSACleanup();
        cout << "                    ";
        cout << "Server closed.\n";
    }
    return 0;
}
