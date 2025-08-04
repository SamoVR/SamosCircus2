#include "Game.h"

Game::Game()
	: window(nullptr), width(1600), height(800), windowTitle("Control Simulator"), core(core)
{

}

Game::~Game() {
	cleanup();
}

///////////

bool Game::init() 
{
	if (!glfwInit()) {
		std::cerr << "Failed to init glfw!\n";
		return false;
	}

	window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create window!\n";
		glfwTerminate();
		return false;
	}

	glfwSetWindowUserPointer(window, this);
	//
	//
	
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to init glew!\n";
		return false;
	}

	core = new Core(0.0f, 0.0f, 0.1f, glm::vec3(0.1f, 0.1f, 0.1f));
	UI = new Interface(window,core);

	return true;
}

void Game::run() {
	if (!init())
		return;

	while (!glfwWindowShouldClose(window)) {
		UI->update();

		core->update();
		core->render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	cleanup();
}

void Game::cleanup() {
	glfwDestroyWindow(window);
	glfwTerminate();
}
