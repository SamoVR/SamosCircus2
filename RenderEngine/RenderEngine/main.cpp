#include "Windows.h"
#include "Engine.h"
#include "resource.h"
#include <iostream>

int main() {
//int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) { // !! THIS IS TO BE UNCOMMENTED IN RELEASE VERSION !!
    HINSTANCE hInstance = GetModuleHandle(nullptr);  // Get current instance handle !! THIS IS TO BE COMMENTED IN RELEASE VERSION !!

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"RIFTWindowClass";
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    RegisterClass(&wc);

    try {
        Engine engine;
        engine.run();
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
