#include "Engine.h"
#include "Windows.h"
#include <iostream>

int main() {
//int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    {
        try {
            Engine engine;   // Create the Engine instance
            engine.run();  // Run the engine loop
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return -1;
        }

        return 0;
    }
}