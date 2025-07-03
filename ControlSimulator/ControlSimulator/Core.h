#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Core {
public:
    Core(float x, float y, float size, glm::vec3 color);

    void update();
    void render();

    float temperature = 100.0f;
    glm::vec3 color;

private:
    float x, y;       // Position
    float size;       // Square size

    const float minTemp = 25.0f;
    const float maxTemp = 1000.0f;
};
