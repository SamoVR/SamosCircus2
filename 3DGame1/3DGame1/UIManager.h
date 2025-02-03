#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class Camera;

class UIManager {
public:
    void Init(GLFWwindow* window,Camera* camera); // Initialize ImGui with the GLFW window
    void UpdateUI();            // Setup and render ImGui frames
    void SetupCoreUI();
    void Cleanup();               // Properly shut down ImGui resources

private:
    bool isInitialized = false;   // Track if ImGui has been initialized
    Camera* camera = nullptr;
    ImFont* headingFont;

    GLuint crosshairTexture = 1;
    int crosshairWidth = 1, crosshairHeight = 1;
};

#endif
