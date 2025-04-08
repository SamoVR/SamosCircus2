#pragma once

#include "Scene.h"
#include "Object.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <glm/gtc/type_ptr.hpp>

class Interface {
public:
    Interface(GLFWwindow* window, Scene& scene);
    ~Interface();

    void createUI();  // Create UI elements for the objects
    void update();    // Update the ImGui frame and render it

private:
    GLFWwindow* window;
    ImFont* headingFont;
    Scene& scene;  // Reference to the current scene

    void displayObjectProperties(Object& object);
    void displayObjectList();
};
