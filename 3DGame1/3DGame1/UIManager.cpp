#include "UIManager.h"
#include "Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

#include "stb_image.h"

void UIManager::Init(GLFWwindow* window, Camera* camera) {
    if (isInitialized) {
        std::cerr << "UI is already initialized!" << std::endl;
        return;
    }

    this->camera = camera;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    SetupCoreUI();

    isInitialized = true;
}

void UIManager::UpdateUI() {
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
    if (ImGui::SliderFloat("Movement Speed", &movementSpeed, 0.0f, 50.0f)) {
        camera->setMovementSpeed(movementSpeed);
    }

    ImGui::End();

    ////////// Crosshair //////////
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    float centerX = io.DisplaySize.x * 0.5f;
    float centerY = io.DisplaySize.y * 0.5f;

    ImGui::SetNextWindowPos(ImVec2(centerX, centerY), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

    ImGui::Begin("Crosshair", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground);

    if (crosshairTexture) {
        float iconSize = 32.0f; // Adjust based on texture atlas size
        ImVec2 uv0 = ImVec2(0.0f, 0.0f);
        ImVec2 uv1 = ImVec2(1.0f / 16.0f, 1.0f / 16.0f); // Assuming a 16x16 atlas

        ImGui::Image((ImTextureID)(intptr_t)crosshairTexture, ImVec2(iconSize, iconSize), uv0, uv1);
    }

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void UIManager::SetupCoreUI() {

    int channels;

    unsigned char* data = stbi_load("assets/textures/icons.png", &crosshairWidth, &crosshairHeight, &channels, 4);
    if (data) {
        glGenTextures(1, &crosshairTexture);
        glBindTexture(GL_TEXTURE_2D, crosshairTexture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, crosshairWidth, crosshairHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load crosshair texture!" << std::endl;
    }


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
