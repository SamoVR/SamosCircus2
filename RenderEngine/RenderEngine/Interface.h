#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Interface {
public:
    Interface(GLFWwindow* window);
    ~Interface();

    void createUI();  // This will handle UI elements like sliders, buttons, etc.
    void update();    // Update the ImGui frame and render it

private:
    GLFWwindow* window;
};
