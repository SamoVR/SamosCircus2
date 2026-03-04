#pragma once
#include "GLFW/glfw3.h"
#include <stdexcept>

class Game
{
	public:
		Game();
		~Game();
		void run();
		GLFWwindow* window;
	private:
		bool init();
		void update();
		void cleanup();
};

