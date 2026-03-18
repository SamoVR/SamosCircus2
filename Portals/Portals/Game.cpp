#include "Game.h"

Game::Game() : window(nullptr)
{
	
}

Game::~Game()
{
	cleanup();
}

void Game::SpawnTestObjects() {
	Cube c;
	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f((c.x /400.0f) - 1.0f, 1.0f - (c.y / 300.0f));
		glVertex2f(((c.x + c.size) / 400.0f) - 1.0f, 1.0f - (c.y / 300.0f));
		glVertex2f(((c.x + c.size) / 400.0f) - 1.0f, 1.0f - ((c.y + c.size)/300.0f));
		glVertex2f((c.x / 400.0f) - 1.0f, 1.0f - ((c.y + c.size) / 300.0f));
	glEnd();
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
		
		deltaTime = (float)glfwGetTime();
		physics.run();
		update(deltaTime);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

void Game::update(float dt)
{
	SpawnTestObjects();
	if (glfwGetKey(window,GLFW_KEY_ESCAPE)) { //esc = exit
		glfwSetWindowShouldClose(window, true);
	}
}

void Game::cleanup() {
	if(window)
		glfwDestroyWindow(window);

	glfwTerminate();
}
