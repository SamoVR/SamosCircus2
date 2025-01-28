#include "Game.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

const int width = 800, height = 600;

Game::Game()
    : camera(glm::vec3(0.0f, 2.0f, 3.0f)), lastFrame(0.0f)
{
    window = initWindow(width, height, "3DGame1", camera); // Initialize the window
    if (!window) {
        throw std::runtime_error("Failed to initialize the window.");
    }
}

Game::~Game() {
    cleanup();
}

void Game::run() {
    init();

    // Main game loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents(); // Handle input events

        update(deltaTime); // Update game logic

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        render();          // Render the scene

        glfwSwapBuffers(window); // Swap buffers to display the next frame
    }
}

void Game::init() {
    uiManager.Init(window); // Initialize ImGui

    // Compile and set shaders
    shaderProgram = renderer.compileShader("vertex.glsl", "fragment.glsl");
    renderer.setShaderProgram(shaderProgram);

    // Initialize room or other game objects
    room.init();

    glEnable(GL_DEPTH_TEST);
}

void Game::update(float deltaTime) {
    inputManager.handleMovement(window, camera, deltaTime);
}

void Game::render() {
    renderer.clear(0.2f, 0.2f, 0.3f, 1.0f);

    // Use the shader program
    glUseProgram(shaderProgram);

    // Setup matrices
    glm::mat4 projection = camera.getProjection(width, height);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Render the room
    room.render();

    // Render ImGui
    uiManager.SetupImgui();
}

void Game::cleanup() {
    uiManager.Cleanup(); // Cleanup ImGui
    glfwDestroyWindow(window); // Destroy the GLFW window
    glfwTerminate();           // Terminate GLFW
}
