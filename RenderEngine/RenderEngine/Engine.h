#pragma once

#define GLM_ENABLE_EXPERIMENTAL

#include "Camera.h"
#include "Object.h"
#include "Scene.h"
#include "Shader.h"
#include "Interface.h"
#include "Geometry.h"
#include "InputManager.h"
#include "KeybindManager.h"

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
    InputManager* inputManager;
    KeybindManager* keybindManager;

    int width;
    int height;

    void run();

private:
    Shader* shader;
    Shader* backgroundShader;
    Shader* gridShader;
    Shader* lineShader;
    Object* targetVisualizer;

    GLFWwindow* window;
    Interface* UI;

    std::string windowTitle;

    int gridLineCount = 0;
    float lastFrame;

    GLuint lineVAO, lineVBO;
    GLuint gridVAO, gridVBO;
    GLuint fullScreenQuadVAO, fullScreenQuadVBO;

    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

    bool init();
    void setupDefaultKeybinds();
    void update(float deltaTime);
    void render(float deltaTime);
    void initLines();
    void initGrid();
    void initFullScreenQuad();
    void drawLine(const glm::vec3& start, const glm::vec3& end, const ImVec4& color);
    void renderGrid();
    void renderBackground();
    void renderMeasurementLines(Object* object);
    void performObjectPicking(double mouseX, double mouseY);
    void processInput();
    void cleanup();
};
