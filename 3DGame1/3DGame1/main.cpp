//
#include "Window.h"
#include "Camera.h"
#include "InputManager.h"
#include "UIManager.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>

// Vertex Shader source
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    ourColor = aColor;  // Pass the color to the fragment shader
}
)";

// Fragment Shader source
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(ourColor, 1.0); // Use the interpolated color
}
)";

// Room vertices (simple cube room)
float roomVertices[] = {
    // Floor (2 triangles)
    -5.0f, -1.0f, -5.0f, 0.7f, 0.7f, 0.7f,  // Grey
     5.0f, -1.0f, -5.0f, 0.7f, 0.7f, 0.7f,
     5.0f, -1.0f,  5.0f, 0.7f, 0.7f, 0.7f,
    -5.0f, -1.0f, -5.0f, 0.7f, 0.7f, 0.7f,
     5.0f, -1.0f,  5.0f, 0.7f, 0.7f, 0.7f,
    -5.0f, -1.0f,  5.0f, 0.7f, 0.7f, 0.7f,

    // Back Wall (2 triangles)
    -5.0f, -1.0f, -5.0f, 0.0f, 0.0f, 1.0f,  // Blue
     5.0f, -1.0f, -5.0f, 0.0f, 0.0f, 1.0f,
     5.0f,  5.0f, -5.0f, 0.0f, 0.0f, 1.0f,
    -5.0f, -1.0f, -5.0f, 0.0f, 0.0f, 1.0f,
     5.0f,  5.0f, -5.0f, 0.0f, 0.0f, 1.0f,
    -5.0f,  5.0f, -5.0f, 0.0f, 0.0f, 1.0f,

    // Front Wall (2 triangles)
    -5.0f, -1.0f,  5.0f, 1.0f, 0.0f, 0.0f,  // Red
     5.0f, -1.0f,  5.0f, 1.0f, 0.0f, 0.0f,
     5.0f,  5.0f,  5.0f, 1.0f, 0.0f, 0.0f,
    -5.0f, -1.0f,  5.0f, 1.0f, 0.0f, 0.0f,
     5.0f,  5.0f,  5.0f, 1.0f, 0.0f, 0.0f,
    -5.0f,  5.0f,  5.0f, 1.0f, 0.0f, 0.0f,

    // Left Wall (2 triangles)
    -5.0f, -1.0f, -5.0f, 0.0f, 1.0f, 0.0f,  // Green
    -5.0f, -1.0f,  5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  5.0f,  5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f, -1.0f, -5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  5.0f,  5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  5.0f, -5.0f, 0.0f, 1.0f, 0.0f,

    // Right Wall (2 triangles)
    10.0f, -1.0f, -5.0f, 1.0f, 0.0f, 1.0f,  // Pink
    10.0f, -1.0f,  5.0f, 1.0f, 0.0f, 1.0f,
    10.0f,  5.0f,  5.0f, 1.0f, 0.0f, 1.0f,
    10.0f, -1.0f, -5.0f, 1.0f, 0.0f, 1.0f,
    10.0f,  5.0f,  5.0f, 1.0f, 0.0f, 1.0f,
    10.0f,  5.0f, -5.0f, 1.0f, 0.0f, 1.0f,

};

const int width = 800, height = 600;

// Compile shader and create program
GLuint compileShader(const char* vertexSrc, const char* fragmentSrc) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Main function
int main() {
    glm::vec3 cameraPos(0.0f, 2.0f, 3.0f);

    Camera camera(cameraPos);
    InputManager inputManager;
    UIManager uiManager;

    GLFWwindow* window = initWindow(width, height, "3DGame1",camera);
    if (!window) return -1;

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    GLuint shaderProgram = compileShader(vertexShaderSource, fragmentShaderSource);

    // Setup room VAO
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomVertices), roomVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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

        glClearColor(0.2, 0.2, 0.3,0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render room
        glUseProgram(shaderProgram);

        glm::mat4 projection = camera.getProjection(width,height);
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 30); // 6 triangles, 18 vertices

        // Render ImGui
        uiManager.SetupImgui();
        

        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
