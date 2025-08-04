#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Core.h"
#include "Interface.h"

class Game
{
public:
	Game();
	~Game();
	
	int width;
	int height;

	float controlRodLevel = 0.0f;

	void run();
	
private:
	GLFWwindow* window;
	Core* core;
	Interface* UI;

	std::string windowTitle;

	bool init();
	void cleanup();
	void renderCube();

};

