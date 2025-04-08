#pragma once

#include <vector>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Texture.h"
#include "Vertex.h"

class Object {
public:
    Object(const std::vector<Vertex>& vertices, Texture* texture = nullptr);
    ~Object();

    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };

    void draw(Shader& shader) const;

    void setPosition(const glm::vec3& pos);
    void setScale(const glm::vec3& scale);
    void setRotation(const glm::vec3& axis);

    glm::mat4 getModelMatrix() const;

private:
    GLuint VAO, VBO;
    size_t vertexCount;
    Texture* texture;

    void setup(const std::vector<Vertex>& vertices);
};
