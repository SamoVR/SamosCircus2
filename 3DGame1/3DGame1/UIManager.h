#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <GLFW/glfw3.h>

class UIManager {
public:
    void Init(GLFWwindow* window); // Initialize ImGui with the GLFW window
    void SetupImgui();            // Setup and render ImGui frames
    void Cleanup();               // Properly shut down ImGui resources

private:
    bool isInitialized = false;   // Track if ImGui has been initialized
};

#endif
