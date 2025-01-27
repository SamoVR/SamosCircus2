#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

class WindowManager {
public:
    GLFWwindow* init(int width, int height, const char* title);
    void beginImGuiFrame();
    void endImGuiFrame();
    ~WindowManager();
};

#endif
