#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "Camera.h"
#include "Object.h"
#include "Scene.h"
#include "Shader.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

class Engine
{
public:
    Engine();
    ~Engine();

    Object* cube;

    void run();

private:
    Shader* shader;
    Camera* camera;
    GLFWwindow* window;
    int width;
    int height;
    std::string windowTitle;

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    bool init();
    void update();
    void render();
    void processInput();
    void cleanup();
};
