#include <iostream>
#include <winsock2.h>
#include <string>
#include <cstdio>

#pragma comment(lib, "ws2_32.lib")

int portDefault = 55665;
int PORT = portDefault;

void pwc(const char msg[], int color) {
    using namespace std;
    cout << "\033[" + to_string(color) + "m" << msg << "\033[0m";
}

void pwc(const char msg[], int color, int bg_color) {
    using namespace std;
    cout << "\033[" + to_string(color) + ";" + to_string(bg_color) + "m" << msg << "\033[0m";
}

void printLog(int err_code) {
    switch (err_code) {
        case 1:
            pwc(" INF ", 30, 47);
            std::cout << " SuccessFul connectection." << std::endl;
            return;
        case 2:
            pwc(" INF ", 30, 47);
            std::cout << " Waiting for connection..." << std::endl;
            return;
        case 3:
            pwc(" INF ", 30, 44);
            pwc(" Disconnection.", 34);
            std::cout << std::endl;
            return;
        case 101:
            pwc(" ERR ", 30, 41);
            pwc(" 101 - Program initialization failure!", 31);
            std::cout << std::endl;
            return;
        case 102:
            pwc(" ERR ", 30, 41);
            pwc(" 102 - Socket binding failed!", 31);
            std::cout << std::endl;
            return;
        case 103:
            pwc(" ERR ", 30, 41);
            pwc(" 103 - Failed to listen for network requests!", 31);
            std::cout << std::endl;
            return;
        case 104:
            pwc(" ERR ", 30, 41);
            pwc(" 104 - Failed to accept client connection!", 31);
            std::cout << std::endl;
            return;
        case 105:
            pwc(" ERR ", 30, 41);
            pwc(" 105 - Illegal port!", 31);
            std::cout << std::endl;
            return;
    }
}

char* execute_command(const char* cmd) {
    FILE* pipe = _popen(cmd, "r");
    if (!pipe) {
        std::cerr << "Error: popen failed!" << std::endl;
        return NULL;
    }
    char buffer[128];
    char* result = new char[1];
    result[0] = '\0';

    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != NULL) {
            char* temp = new char[strlen(result) + strlen(buffer) + 1];
            strcpy(temp, result);
            strcat(temp, buffer);
            delete[] result;
            result = temp;
        }
    }
    _pclose(pipe);
    return result;
}


int main() {
    while(1){
        char port_str[6];
        int port;
        std::cout << "Setting the port: ";
        std::cin.getline(port_str, 6);
        if(strlen(port_str) == 0) {
            std::cout << "\033[1ASetting the port: " << portDefault << std::endl;
            break;
        }else{
            port = std::atoi(port_str);
            if(port > 0 && port < 65535) {
                PORT = port;
                break;
            }else{
                printLog(105);
            }
        }
    }

    while(1) {
        // 初始化
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            printLog(101);
            return -1;
        }
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (serverSocket == INVALID_SOCKET) {
            printLog(101);
            WSACleanup();
            return -1;
        }
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            printLog(102);
            closesocket(serverSocket);
            WSACleanup();
            return -1;
        }

        // 监听
        if (listen(serverSocket, 5) == SOCKET_ERROR) {
            printLog(103);
            closesocket(serverSocket);
            WSACleanup();
            return -1;
        }

        // 连接
        SOCKET clientSocket;
        sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        printLog(2);
        clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            printLog(104);
            closesocket(serverSocket);
            WSACleanup();
            return -1;
        }else{
            printLog(1);
        }

        // 通信
        char* cmdOutput = nullptr; 
        while (1) {
            char buffer[1024];
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived == SOCKET_ERROR) {
                printLog(3);
                break;
            } else {
                buffer[bytesReceived] = '\0';
                std::cout << "[Client] " << buffer << std::endl;

                if (cmdOutput != nullptr) {
                    delete[] cmdOutput;
                }

                cmdOutput = execute_command(buffer);
                if(strlen(cmdOutput) == 0){
                    char cmdNull[] = "NULL\n";
                    send(clientSocket, cmdNull, strlen(cmdNull), 0);
                }else{
                    send(clientSocket, cmdOutput, strlen(cmdOutput), 0);
                }
            }
        }

        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();

        std::cout << std::endl;
    }
    return 0;
}
