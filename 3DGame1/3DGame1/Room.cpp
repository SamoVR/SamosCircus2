#include "Room.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Room vertices (a simple cube room)
float roomVertices[] = {
    // Floor (2 triangles)
    -5.0f, -1.0f, -5.0f, 0.7f, 0.7f, 0.7f,  // Grey
     5.0f, -1.0f, -5.0f, 0.7f, 0.7f, 0.7f,
     5.0f, -1.0f,  5.0f, 0.7f, 0.7f, 0.7f,
    -5.0f, -1.0f, -5.0f, 0.7f, 0.7f, 0.7f,
     5.0f, -1.0f,  5.0f, 0.7f, 0.7f, 0.7f,
    -5.0f, -1.0f,  5.0f, 0.7f, 0.7f, 0.7f,

    // Back Wall (2 triangles)
    -5.0f, -1.0f, -5.0f, 0.0f, 0.0f, 1.0f,  // Blue
     5.0f, -1.0f, -5.0f, 0.0f, 0.0f, 1.0f,
     5.0f,  5.0f, -5.0f, 0.0f, 0.0f, 1.0f,
    -5.0f, -1.0f, -5.0f, 0.0f, 0.0f, 1.0f,
     5.0f,  5.0f, -5.0f, 0.0f, 0.0f, 1.0f,
    -5.0f,  5.0f, -5.0f, 0.0f, 0.0f, 1.0f,

    // Front Wall (2 triangles)
    -5.0f, -1.0f,  5.0f, 1.0f, 0.0f, 0.0f,  // Red
     5.0f, -1.0f,  5.0f, 1.0f, 0.0f, 0.0f,
     5.0f,  5.0f,  5.0f, 1.0f, 0.0f, 0.0f,
    -5.0f, -1.0f,  5.0f, 1.0f, 0.0f, 0.0f,
     5.0f,  5.0f,  5.0f, 1.0f, 0.0f, 0.0f,
    -5.0f,  5.0f,  5.0f, 1.0f, 0.0f, 0.0f,

    // Left Wall (2 triangles)
    -5.0f, -1.0f, -5.0f, 0.0f, 1.0f, 0.0f,  // Green
    -5.0f, -1.0f,  5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  5.0f,  5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f, -1.0f, -5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  5.0f,  5.0f, 0.0f, 1.0f, 0.0f,
    -5.0f,  5.0f, -5.0f, 0.0f, 1.0f, 0.0f,

    // Right Wall (2 triangles)
    10.0f, -1.0f, -5.0f, 1.0f, 0.0f, 1.0f,  // Pink
    10.0f, -1.0f,  5.0f, 1.0f, 0.0f, 1.0f,
    10.0f,  5.0f,  5.0f, 1.0f, 0.0f, 1.0f,
    10.0f, -1.0f, -5.0f, 1.0f, 0.0f, 1.0f,
    10.0f,  5.0f,  5.0f, 1.0f, 0.0f, 1.0f,
    10.0f,  5.0f, -5.0f, 1.0f, 0.0f, 1.0f,

    // Floor2 (2 triangles)
    -10.0f, -1.0f, -30.0f, 0.0f, 0.7f, 0.0f,  // Dark Green
     10.0f, -1.0f, -30.0f, 0.0f, 0.7f, 0.0f,
     10.0f, -1.0f, -10.0f, 0.0f, 0.7f, 0.0f,
    -10.0f, -1.0f, -30.0f, 0.0f, 0.7f, 0.0f,
     10.0f, -1.0f, -10.0f, 0.0f, 0.7f, 0.0f,
    -10.0f, -1.0f, -10.0f, 0.0f, 0.7f, 0.0f,

};

Room::Room() : VAO(0), VBO(0) {}

Room::~Room() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Room::init() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomVertices), roomVertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Room::render() {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36); // Update with actual vertex count
    glBindVertexArray(0);
}
