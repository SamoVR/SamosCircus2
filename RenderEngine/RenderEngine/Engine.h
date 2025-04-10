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

    Camera* camera;
    Object* cube;
    Scene scene;

    int width;
    int height;

    void run();

private:
    Shader* shader;
    Shader* backgroundShader;
    Shader* gizmoShader;
    Object* targetVisualizer;

    GLFWwindow* window;
    Interface* UI;

    std::string windowTitle;

    float lastFrame;

    GLuint fullScreenQuadVAO, fullScreenQuadVBO;
    GLuint gizmoVAO, gizmoVBO;

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    bool init();
    void update(float deltaTime);
    void render();
    void initGizmoLines();
    void initFullScreenQuad();
    void renderGizmoForObject(Object* object);
    bool castRay(double mouseX, double mouseY, Object*& selectedObject);
    void processGizmoInteraction(double mouseX, double mouseY);
    void renderBackground();
    void processInput();
    void cleanup();
};
