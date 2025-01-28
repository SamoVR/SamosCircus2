//
#include "Window.h"
#include "Camera.h"
#include "InputManager.h"
#include "UIManager.h"
#include "Renderer.h"
#include "Room.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

const int width = 800, height = 600;

// Main function
int main() {
    glm::vec3 cameraPos(0.0f, 2.0f, 3.0f);

    Camera camera(cameraPos);
    InputManager inputManager;
    UIManager uiManager;
    Renderer renderer;
    Room room;

    GLFWwindow* window = initWindow(width, height, "3DGame1",camera);
    if (!window) return -1;

    uiManager.Init(window);

    GLuint shaderProgram = renderer.compileShader("vertex.glsl","fragment.glsl");
    renderer.setShaderProgram(shaderProgram);

    room.init();

    glEnable(GL_DEPTH_TEST);

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();

        inputManager.handleMovement(window,camera,deltaTime);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        renderer.clear(0.2f, 0.2f, 0.3f, 1.0f);

        // Render room
        glUseProgram(shaderProgram);

        glm::mat4 projection = camera.getProjection(width, height);
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        room.render();

        // Render ImGui
        uiManager.SetupImgui();
        
        glfwSwapBuffers(window);
    }

    // Cleanup

    uiManager.Cleanup();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
