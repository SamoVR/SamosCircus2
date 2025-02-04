#include "ColliderVisualizer.h"
#include <glm/gtc/type_ptr.hpp>

GLuint ColliderVisualizer::VAO = 0;
GLuint ColliderVisualizer::VBO = 0;

void ColliderVisualizer::setup() {
    GLfloat vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,
        // Back face
        -0.5f, -0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
        // Connecting edges
        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f,  0.5f,
        0.5f, -0.5f, -0.5f,  0.5f, -0.5f,  0.5f,
        0.5f,  0.5f, -0.5f,  0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f, -0.5f,  0.5f,  0.5f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ColliderVisualizer::render(const glm::mat4& modelMatrix, GLuint shaderProgram, const glm::vec3& colliderMin, const glm::vec3& colliderMax) {
    // Define the collider vertices
    GLfloat colliderVertices[] = {
        // Front face
        colliderMin.x, colliderMin.y, colliderMax.z, colliderMax.x, colliderMin.y, colliderMax.z,
        colliderMax.x, colliderMin.y, colliderMax.z, colliderMax.x, colliderMax.y, colliderMax.z,
        colliderMax.x, colliderMax.y, colliderMax.z, colliderMin.x, colliderMax.y, colliderMax.z,
        colliderMin.x, colliderMax.y, colliderMax.z, colliderMin.x, colliderMin.y, colliderMax.z,

        // Back face
        colliderMin.x, colliderMin.y, colliderMin.z, colliderMax.x, colliderMin.y, colliderMin.z,
        colliderMax.x, colliderMin.y, colliderMin.z, colliderMax.x, colliderMax.y, colliderMin.z,
        colliderMax.x, colliderMax.y, colliderMin.z, colliderMin.x, colliderMax.y, colliderMin.z,
        colliderMin.x, colliderMax.y, colliderMin.z, colliderMin.x, colliderMin.y, colliderMin.z,

        // Connecting edges
        colliderMin.x, colliderMin.y, colliderMin.z, colliderMin.x, colliderMin.y, colliderMax.z,
        colliderMax.x, colliderMin.y, colliderMin.z, colliderMax.x, colliderMin.y, colliderMax.z,
        colliderMax.x, colliderMax.y, colliderMin.z, colliderMax.x, colliderMax.y, colliderMax.z,
        colliderMin.x, colliderMax.y, colliderMin.z, colliderMin.x, colliderMax.y, colliderMax.z
    };

    glLineWidth(4.0f);  // Make lines thicker

    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colliderVertices), colliderVertices, GL_STATIC_DRAW);
    glDrawArrays(GL_LINES, 0, 24);  // 24 vertices for the wireframe cube

    glBindVertexArray(0);
    glLineWidth(1.0f);  // Reset line width
}
