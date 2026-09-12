#pragma once
#include "GLFW/glfw3.h"
#include <stdexcept>
#include <vector>
#include "Physics.h"
#include "Cube.h"

class Game
{
	public:
		Game();
		~Game();
		void run();
		GLFWwindow* window;
		std::vector<Cube> objects;

	private:
		bool init();
		void update(float dt);
		void cleanup();
		void SpawnTestObjects();
		Physics physics;
		float deltaTime;

};

