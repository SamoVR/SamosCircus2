#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 54000
#define SERVER_IP "127.0.0.1"  // Change this to your server's IP

void receiveMessages(SOCKET clientSocket) {
    char buffer[4096];

    while (true) {
        ZeroMemory(buffer, 4096);
        int bytesReceived = recv(clientSocket, buffer, 4096, 0);
        if (bytesReceived <= 0) {
            std::cout << "Disconnected from server.\n";
            break;
        }

        std::cout << "\n" << std::string(buffer, bytesReceived) << std::endl;
    }

    closesocket(clientSocket);
    WSACleanup();
    exit(0);
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed!" << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed!" << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to the server! Type messages and press ENTER to send.\n";

    std::thread recvThread(receiveMessages, clientSocket);
    recvThread.detach();

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        if (message == "/exit") break;

        send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
