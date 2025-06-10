#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

//* GLOBALS *//

int cubeAmount = 20;
bool randomizeCoords = false;
bool rotateGradually = true;
bool loop = false;

//**********//

struct Cube {
	float x, y;
	float size;
	float rotation;
	glm::vec3 color;
	bool gravity;
	bool bounce;

	Cube(float x, float y, float size = 1.0f, float rotation = 0.0f, glm::vec3 color = glm::vec3(0.0f,0.0f,0.0f), bool gravity = true, bool bounce = false)
		: x(x), y(y), size(size), rotation(rotation), color(color), gravity(gravity), bounce(bounce) {}

	void draw() const {
		glPushMatrix(); // save current transform

		glTranslatef(x, y, 0.0f);
		glRotatef(rotation, 0.0f, 0.0f, 1.0f);
		glScalef(size, size, 1.0f);

		glBegin(GL_QUADS);
		glColor3f(color.x,color.y,color.z);
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
		Cube cube(0.0f, 0.0f, 0.5f, r, glm::vec3(1.0f, 0.0f, 0.0f),false);

		if (randomizeCoords) {
			cube.x = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
			cube.y = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
		}

		cubes.push_back(cube);

		if (rotateGradually)
			r += 10.0f;

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	std::cout << "finished generating" << std::endl;
	if(loop){
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		cubes = {};
		generateCubes();
	}
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

	/*std::thread cubeGenThread(generateCubes);
	cubeGenThread.detach();*/

	Cube cube(0.0f, 0.0f, 0.25f, 0.0f,glm::vec3(1.0f,0.0f,0.0f),false,true);
	cubes.push_back(cube);

	Cube floor(0.0f,-2.0f,2.1425f, 0.0f, glm::vec3(0.3, 0.3, 0.3), false);

	bool upsidedown = false;

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);
		glLoadIdentity();

		// RENDERING //

		floor.draw();

		for (int i = 0; i < cubes.size(); i++) {
			cubes[i].draw();
		}

		// END RENDERING //

		// GRAVITY TEST //

		for (int i = 0; i < cubes.size(); i++) {
			if(cubes[i].gravity) {
				if (cubes[i].y > cubes[i].size - 1.0f)
					cubes[i].y -= 0.01f;
				else
					cubes[i].y = cubes[i].size - 1.0f;
			}
		}

		//

		// BOUNCE TEST //

		for (int i = 0; i < cubes.size(); i++) {
			if (cubes[i].bounce && !cubes[i].gravity) {
				if (cubes[i].y > cubes[i].size - 1.0f && !upsidedown) {
					cubes[i].y -= 0.001f;
				}
				else {
					upsidedown = true;
					cubes[i].y += 0.001f;
				}
				if (cubes[i].y < cubes[i].size + 1.0f && upsidedown) {
					cubes[i].y += 0.001f;
				}
				else if(upsidedown) {
					upsidedown = false;
					cubes[i].y -= 0.001f;
				}
				if (cubes[i].x < cubes[i].size + 1.0f) {
					cubes[i].x += 0.001f;
				}
			}
		}

		//

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}