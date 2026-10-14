// Entry points.
//
// Both `main` and `wWinMain` are provided, so the project's Debug configurations
// (SubSystem=Console) and Release|x64 (SubSystem=Windows) each link against the
// matching one WITHOUT editing this file. The original required commenting one
// signature in and the other out by hand before every release build
// ("!! THIS IS TO BE UNCOMMENTED IN RELEASE VERSION !!").
//
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <shellapi.h>
#endif

#include "Engine.h"
#include "Logger.h"
#include <exception>
#include <iostream>

namespace {

    // Start-up failures used to be printed to std::cerr, which does not exist in
    // a Windows-subsystem Release build, so the program just vanished. Show the
    // reason in a dialog and keep it in the log file.
    void reportFatalError(const char* message) {
        LOG_ERROR("Main", std::string("Fatal error: ") + message);
#ifdef _WIN32
        MessageBoxA(nullptr, message, "RIFT Engine - fatal error", MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
#else
        std::cerr << "Fatal error: " << message << std::endl;
#endif
    }

    int runApplication(const std::string& startupScene) {
        try {
            Engine engine;
            return engine.run(startupScene);
        }
        catch (const std::exception& e) {
            reportFatalError(e.what());
        }
        catch (...) {
            reportFatalError("An unknown error occurred.");
        }
        return -1;
    }
}

int main(int argc, char** argv) {
    return runApplication(argc > 1 ? argv[1] : "");
}

#ifdef _WIN32
int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    // First argument (UTF-8), e.g. a .json scene dropped on the exe.
    std::string startupScene;
    int argc = 0;
    if (LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc)) {
        if (argc > 1) {
            const int size = WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, nullptr, 0, nullptr, nullptr);
            if (size > 1) {
                startupScene.resize(static_cast<size_t>(size) - 1);
                WideCharToMultiByte(CP_UTF8, 0, argv[1], -1, startupScene.data(), size, nullptr, nullptr);
            }
        }
        LocalFree(argv);
    }
    return runApplication(startupScene);
}
#endif
