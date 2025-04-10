#pragma once

#include "Scene.h"
#include "Object.h"
#include "Vertex.h"
#include "Geometry.h"
#include "Texture.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <vector>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include "ImGuiFileDialog-master/ImGuiFileDialog.h"

class Interface {
public:
    Interface(GLFWwindow* window, Scene& scene);
    ~Interface();

    void update();    // Update the ImGui frame and render it

private:
    GLFWwindow* window;
    ImFont* headingFont;
    Scene& scene;  // Reference to the current scene
    Texture* defaultTexture;
    Object* textureTargetObject = nullptr;


    bool showShapePopup = false;
    void displayObjectProperties(Object* object, int index);
    void sceneControlsUI();
    void objectListUI();
    void settingsUI();
    void showShapeSelectionPopup();
    std::string selectedTexturePath = "assets/textures/texture_08.png";
};
