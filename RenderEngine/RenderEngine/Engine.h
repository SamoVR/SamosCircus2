#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "Camera.h"
#include "Object.h"
#include "Scene.h"
#include "Shader.h"
#include "Interface.h"
#include "Geometry.h"

#include "stb_image.h"
#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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
    Scene scene;

    void run();

private:
    Shader* shader;
    Shader* backgroundShader;
    Camera* camera;
    GLFWwindow* window;
    Interface* UI;

    int width;
    int height;
    std::string windowTitle;

    float lastFrame;

    GLuint fullScreenQuadVAO, fullScreenQuadVBO;

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    bool init();
    void update(float deltaTime);
    void render();
    void initFullScreenQuad();
    void renderBackground();
    void processInput();
    void cleanup();
};
