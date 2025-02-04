#include "ColliderVisualizer.h"
#include <glm/gtc/type_ptr.hpp>

GLuint ColliderVisualizer::VAO = 0;
GLuint ColliderVisualizer::VBO = 0;

void ColliderVisualizer::setup() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
}

void ColliderVisualizer::render(const glm::mat4& modelMatrix, GLuint shaderProgram, const glm::vec3& colliderMin, const glm::vec3& colliderMax) {
    // Define the 8 corners of the bounding box
    GLfloat colliderVertices[] = {
        // Bottom square (Y = colliderMin.y)
        colliderMin.x, colliderMin.y, colliderMin.z,
        colliderMax.x, colliderMin.y, colliderMin.z,

        colliderMax.x, colliderMin.y, colliderMin.z,
        colliderMax.x, colliderMin.y, colliderMax.z,

        colliderMax.x, colliderMin.y, colliderMax.z,
        colliderMin.x, colliderMin.y, colliderMax.z,

        colliderMin.x, colliderMin.y, colliderMax.z,
        colliderMin.x, colliderMin.y, colliderMin.z,

        // Top square (Y = colliderMax.y)
        colliderMin.x, colliderMax.y, colliderMin.z,
        colliderMax.x, colliderMax.y, colliderMin.z,

        colliderMax.x, colliderMax.y, colliderMin.z,
        colliderMax.x, colliderMax.y, colliderMax.z,

        colliderMax.x, colliderMax.y, colliderMax.z,
        colliderMin.x, colliderMax.y, colliderMax.z,

        colliderMin.x, colliderMax.y, colliderMax.z,
        colliderMin.x, colliderMax.y, colliderMin.z,

        // Vertical edges
        colliderMin.x, colliderMin.y, colliderMin.z,
        colliderMin.x, colliderMax.y, colliderMin.z,

        colliderMax.x, colliderMin.y, colliderMin.z,
        colliderMax.x, colliderMax.y, colliderMin.z,

        colliderMax.x, colliderMin.y, colliderMax.z,
        colliderMax.x, colliderMax.y, colliderMax.z,

        colliderMin.x, colliderMin.y, colliderMax.z,
        colliderMin.x, colliderMax.y, colliderMax.z
    };

    glUseProgram(shaderProgram);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colliderVertices), colliderVertices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 24);
    glLineWidth(1.0f);

    glBindVertexArray(0);
}
