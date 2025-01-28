#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Camera;

class UIManager {
public:
    void Init(GLFWwindow* window,Camera* camera); // Initialize ImGui with the GLFW window
    void SetupImgui();            // Setup and render ImGui frames
    void Cleanup();               // Properly shut down ImGui resources

private:
    bool isInitialized = false;   // Track if ImGui has been initialized
    Camera* camera = nullptr;
};

#endif
