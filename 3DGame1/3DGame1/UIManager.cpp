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

    ImGui::Begin("Debug Controls");
    //ImGui::PushFont(headingFont);
    ImGui::Text("Player Controls");
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
