#include <iostream>
#include <winsock2.h>
#include <string>
#include <regex>

#pragma comment(lib, "ws2_32.lib")

char IP[16];
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
            std::cout << " Connecting..." << std::endl;
            return;
        case 101:
            pwc(" ERR ", 30, 41);
            pwc(" 101 - Program initialization failure!", 31);
            std::cout << std::endl;
            return;
        case 201:
            pwc(" ERR ", 30, 41);
            pwc(" 201 - Server connection failed!", 31);
            std::cout << std::endl;
            return;
        case 202:
            pwc(" ERR ", 30, 41);
            pwc(" 202 - Failed to send!", 31);
            std::cout << std::endl;
            return;
        case 203:
            pwc(" ERR ", 30, 41);
            pwc(" 203 - Inaccurate address! [ip]/[ip]:[port]", 31);
            std::cout << std::endl;
            return;
    }
}

bool isAddress(const char* str, const int port) {
    std::regex ipv4_regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return (std::regex_match(str, ipv4_regex) && port > 0 && port < 65535);
}

bool isAddress(const char* str) {
    std::regex ipv4_regex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return std::regex_match(str, ipv4_regex);
}

int main() {
    while(1){
        PORT = portDefault;

        char address[21] = {'\0'};
        std::cout << "Connect to: ";
        std::cin.getline(address, 21);

        char* colonPos = std::strchr(address, ':');
        if (colonPos == nullptr) {
            if(isAddress(address)) {
                strcpy(IP, address);
                break;
            }
            else printLog(203);
        }else{
            std::strncpy(IP, address, colonPos - address);
            IP[colonPos - address] = '\0';

            int i = colonPos - address + 1;
            int j = 0;
            char port[6] = {'\0'};
            while (address[i] != '\0') {
                port[j++] = address[i++];
            }
            PORT = std::atoi(port);
            if(isAddress(IP, PORT)) break;
            else printLog(203);
        }
    }

    // 初始化
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printLog(101);
        return -1;
    }
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        printLog(101);
        WSACleanup();
        return -1;
    }


    // 设置地址
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP);


    // 连接
    printLog(2);
    int connectCount = 1;
    int max_connectCount = 20;
    while(1) {
        if(connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            if(connectCount == 1) printLog(201);
            std::cout << "\033[G";
            pwc(" INF ", 30, 47);
            std::cout << " Retrying " << connectCount << "/" << max_connectCount;
            connectCount++;
            Sleep(100);
            if(connectCount >= max_connectCount) {
                closesocket(clientSocket);
                WSACleanup();
                return -1;
            }
        }else{
            if(connectCount >= 2)std::cout << std::endl;
            printLog(1);
            break;
        }
    }


    // 通信
    char* confirmationMessage = nullptr;
    while(1){
        char buffer[] = {0};
        std::cout << "→ ";
        std::cin.getline(buffer, 1024);
        if(strlen(buffer) == 0) continue;
        const char* message = buffer;
        send(clientSocket, message, strlen(message), 0);

        if (confirmationMessage != nullptr) {
            delete[] confirmationMessage;
        }

        char confirmationMessage[2048];
        int bytesReceived = recv(clientSocket, confirmationMessage, sizeof(confirmationMessage), 0);
        if (bytesReceived == SOCKET_ERROR) {
            printLog(202);
        }else{
            confirmationMessage[bytesReceived] = '\0';
            std::cout << "[Server] " << confirmationMessage;
        }
    }

    // 退出
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
