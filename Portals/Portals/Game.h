#pragma once
#include "GLFW/glfw3.h"
#include <stdexcept>

class Game
{
	public:
		Game();
		~Game();
		void Update();
		GLFWwindow* window;
	private:
		bool init();
};

