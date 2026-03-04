#include "Game.h"

Game::Game() : window(nullptr)
{
	
}

Game::~Game()
{
	cleanup();
}

bool Game::init() {
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	window = glfwCreateWindow(800, 600, "Portals", nullptr, nullptr);
	if (!window)
	{
		throw std::runtime_error("Failed to create GLFW window");
		return false;
	}

	glfwMakeContextCurrent(window);
	glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

	return true;
}

void Game::run() {
	if (!init())
		throw std::runtime_error("Failed to initialize game");

	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT);

		update();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Game::update()
{
	if (glfwGetKey(window,GLFW_KEY_ESCAPE)) { //esc = exit
		glfwSetWindowShouldClose(window, true);
	}
}

void Game::cleanup() {
	if(window)
		glfwDestroyWindow(window);

	glfwTerminate();
}
