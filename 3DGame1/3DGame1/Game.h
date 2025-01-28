#ifndef GAME_H
#define GAME_H

#include "Camera.h"
#include "InputManager.h"
#include "Renderer.h"
#include "UIManager.h"
#include "Room.h"
#include "Window.h"

class Game {
public:
    Game();  // Constructor to initialize the game
    ~Game(); // Destructor to clean up resources

    void run(); // Starts the game loop

private:
    void init();    // Initializes all game systems
    void update(float deltaTime); // Updates the game state
    void render();  // Renders the game scene
    void cleanup(); // Cleans up resources when the game ends

    // Game components
    Camera camera;
    InputManager inputManager;
    Renderer renderer;
    UIManager uiManager;
    Room room;

    GLFWwindow* window;
    GLuint shaderProgram;

    float lastFrame; // Keeps track of the previous frame time
};

#endif
