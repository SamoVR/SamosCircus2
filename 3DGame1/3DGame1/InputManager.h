#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class InputManager {

public:
	void handleMovement(GLFWwindow* window, Camera& camera, float deltaTime);


private:



};

#endif