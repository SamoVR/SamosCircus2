#ifndef WINDOW_H
#define WINDOW_H

#include "Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

GLFWwindow* initWindow(int width, int height, const char* title, Camera& camera);

#endif
