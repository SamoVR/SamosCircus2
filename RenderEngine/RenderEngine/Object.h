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
#include "Camera.h"
#include <nlohmann/json.hpp>

class Object {
public:
    Object(std::string name, const std::vector<Vertex>& vertices, Texture* texture = nullptr);
    ~Object();

    std::string name = "Obj";
    std::vector<Vertex> vertices;  // Store vertices data here
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    glm::vec3 color;
    Texture* texture;

    void draw(Shader& shader,Camera& camera) const;

    void setPosition(const glm::vec3& pos);
    void setScale(const glm::vec3& scale);
    void setRotation(const glm::vec3& axis);
    void setColor(const glm::vec3& newColor);
    void setTexture(Texture* newTexture);
    void removeTexture();
    void setVertices(const std::vector<Vertex>& newVertices);
    std::vector<Vertex> fromJSONToVertices(const nlohmann::json& jsonVertices);

    glm::mat4 getModelMatrix() const;

    nlohmann::json toJSON() const;
    void fromJSON(const nlohmann::json& j);

private:
    GLuint VAO, VBO;
    size_t vertexCount;

    void setup(const std::vector<Vertex>& vertices);
};
