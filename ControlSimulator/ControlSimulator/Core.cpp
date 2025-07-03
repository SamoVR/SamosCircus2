#include "Core.h"

Core::Core(float x = 0.0f, float y = 0.0f, float size = 1.0f, glm::vec3 color = glm::vec3(0.1f,0.1f,0.1f))
	: x(x), y(y), size(size), color(color), temperature(temperature)
{
}

void Core::update() {

}

void Core::render() {
    glPushMatrix();
    glBegin(GL_QUADS);
    glColor3f(color.x, color.y, color.z);
    glVertex2f(x - size, y - size);
    glVertex2f(x + size, y - size);
    glVertex2f(x + size, y + size);
    glVertex2f(x - size, y + size);
    glEnd();
}
