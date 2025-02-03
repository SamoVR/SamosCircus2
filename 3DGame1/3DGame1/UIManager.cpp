#include "UIManager.h"
#include "Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

void UIManager::Init(GLFWwindow* window, Camera* camera) {
    if (isInitialized) {
        std::cerr << "ImGui is already initialized!" << std::endl;
        return;
    }

    this->camera = camera;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    int screen_width = mode->width;
    int screen_height = mode->height;

    const int base_width = 1920;
    const int base_height = 1080;

    float dpi_scale = (float)screen_width / base_width;

    io.FontGlobalScale = dpi_scale;
    
    ////
    
    io.Fonts->AddFontDefault();

    this->headingFont = io.Fonts->AddFontFromFileTTF("assets/fonts/ProggyVector-Regular.ttf", 18.0f);
    if (headingFont == nullptr)
    {
        std::cerr << "Failed to load heading font." << std::endl;
    }

    ImGui_ImplOpenGL3_CreateFontsTexture();

    isInitialized = true;
}

void UIManager::SetupImgui() {
    if (!isInitialized) {
        std::cerr << "ImGui is not initialized! Call Init() first." << std::endl;
        return;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Menu");

    ImGui::PushFont(headingFont);
    ImGui::Text("Debug Info");
    ImGui::PopFont();

    ImGui::Text("XYZ: (%.0f, %.0f, %.0f)", camera->position.x, camera->position.y, camera->position.z);

    ImGui::Separator();

    ImGui::PushFont(headingFont);
    ImGui::Text("Player Controls");
    ImGui::PopFont();

    ImGui::Separator();

    float movementSpeed = camera->getMovementSpeed();
    if (ImGui::SliderFloat("Movement Speed", &movementSpeed, 0.0f, 50.0f))
    {
        camera->setMovementSpeed(movementSpeed);
    }
    
    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void UIManager::Cleanup() {
    if (!isInitialized) {
        std::cerr << "ImGui was not initialized or already cleaned up." << std::endl;
        return;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    isInitialized = false;
}
