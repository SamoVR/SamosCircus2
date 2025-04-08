#include "Interface.h"
#include <iostream>
#include <string>
#include <glm/gtc/type_ptr.hpp>

Interface::Interface(GLFWwindow* window) : window(window) {
    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");
}

Interface::~Interface() {
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void Interface::update() {
    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create the UI
    createUI();

    // End the ImGui frame
    ImGui::Render();
}

void Interface::createUI() {
    ImGui::Begin("Object Controls");

    // Example UI for modifying position, rotation, and size of an object
    static glm::vec3 objectPosition(0.0f, 0.0f, 0.0f);
    static float objectRotation = 0.0f;
    static float objectSize = 1.0f;

    // Position slider
    ImGui::Text("Position");
    if (ImGui::SliderFloat3("Position", glm::value_ptr(objectPosition), -10.0f, 10.0f)) {
        // Apply changes to the object position here, if you have an object class
        // e.g., object.updatePosition(objectPosition);
    }

    // Rotation slider
    ImGui::Text("Rotation");
    if (ImGui::SliderFloat("Rotation", &objectRotation, 0.0f, 360.0f)) {
        // Apply changes to the object rotation here
        // e.g., object.updateRotation(objectRotation);
    }

    // Size slider
    ImGui::Text("Size");
    if (ImGui::SliderFloat("Size", &objectSize, 0.1f, 3.0f)) {
        // Apply changes to the object size here
        // e.g., object.updateSize(objectSize);
    }

    ImGui::End();
}
