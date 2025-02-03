#include "Game.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

const int width = 1200, height = 800;

Game::Game()
    : camera(glm::vec3(0.0f, camera.standHeight, 3.0f)), lastFrame(0.0f)
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
    uiManager.Init(window,&camera); // Initialize ImGui

    // Compile and set shaders
    shaderProgram = renderer.compileShader("vertex.glsl", "fragment.glsl");
    renderer.setShaderProgram(shaderProgram);

    BlockFactory::init();

    //World generation
    glEnable(GL_CULL_FACE); // Cull back faces
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // Counter-clockwise vertex order

    world.generateChunks(1);
    //room.init();

    world.placeBlock(0, 2, 0, BlockType::GRASS);
    world.placeBlock(3, 2, 0, BlockType::DIRT);
    world.placeBlock(6, 2, 0, BlockType::STONE);
    world.placeBlock(9, 2, 0, BlockType::BEDROCK);

    glEnable(GL_DEPTH_TEST);
}

void Game::update(float deltaTime) {
    inputManager.handleKeyboard(window, camera, deltaTime);

    inputManager.handleMouse(window);
}

void Game::render() {
    renderer.clear(0.05f, 0.6f, 1.0f, 1.0f);

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

    //room.render();
    renderer.renderChunks(world);

    // Render ImGui
    uiManager.SetupImgui();
}

void Game::cleanup() {
    uiManager.Cleanup(); // Cleanup ImGui
    glfwDestroyWindow(window); // Destroy the GLFW window
    glfwTerminate();           // Terminate GLFW
}
