#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "Camera.h"
#include "Player.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class InputManager {

public:
	void handleKeyboard(GLFWwindow* window, Camera& camera, float deltaTime);
	void handleMouse(GLFWwindow* window, Player& player);

private:
	bool cursorLocked = true;


};

#endif