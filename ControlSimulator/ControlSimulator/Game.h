#pragma once

#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Core.h"

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

	std::string windowTitle;

	bool init();
	void cleanup();
	void renderCube();

};

