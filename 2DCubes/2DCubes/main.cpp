#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include <GLFW/glfw3.h>

//* GLOBALS *//

int cubeAmount = 100;


//**********//

struct Cube {
	float x, y;
	float size;
	float rotation;

	Cube(float x, float y, float size = 1.0f, float rotation = 0.0f)
		: x(x), y(y), size(size), rotation(rotation) {}

	void draw() const {
		glPushMatrix(); // save current transform

		glTranslatef(x, y, 0.0f);
		glRotatef(rotation, 0.0f, 0.0f, 1.0f);
		glScalef(size, size, 1.0f);

		glBegin(GL_QUADS);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f(0.5f, -0.5f);
		glVertex2f(0.5f, 0.5f);
		glVertex2f(-0.5f, 0.5f);
		glEnd();

		glPopMatrix();

	}

};

std::vector<Cube> cubes = {};

void generateCubes() {
	float r = 0.0f;
	for (int i = 0; i < cubeAmount; i++) {
		Cube cube(0.0f, 0.0f, 0.5f, r);
		cubes.push_back(cube);
		r += 1.0f;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	std::cout << "finished generating" << std::endl;
}

int main() {

	if (!glfwInit()) {
		std::cerr << "Failed to init glfw!" <<std::endl;
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(800, 600, "2D Cubes", NULL, NULL);
	if (!window) {
		glfwTerminate();
		std::cerr << "Failed to init window!" << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);

	srand(std::time(NULL));

	std::thread cubeGenThread(generateCubes);
	cubeGenThread.detach();

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

		// RENDERING //

		for (int i = 0; i < cubes.size(); i++) {
			cubes[i].draw();
		}

		// END RENDERING //

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}