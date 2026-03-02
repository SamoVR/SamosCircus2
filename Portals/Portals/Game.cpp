#include "Game.h"

Game::Game() : window(nullptr)
{
	
}

Game::~Game()
{
	
}

bool Game::init() {
	if (!glfwInit()){throw std::runtime_error("Failed to initialize GLFW");}
	window = glfwCreateWindow(800, 600, "Portals", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window");
	}
	glfwMakeContextCurrent(window);
}

void Game::Update()
{
	if (!init())
		throw std::runtime_error("Failed to initialize game");

	while(!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
}
