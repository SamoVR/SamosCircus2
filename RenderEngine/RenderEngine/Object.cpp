#include "Object.h"

Object::Object(std::string name, const std::vector<Vertex>& vertices, Texture* texture)
    : name(name), vertexCount(vertices.size()), texture(texture)
{
    setup(vertices);
}

Object::~Object() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

void Object::setup(const std::vector<Vertex>& vertices) {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);
}

glm::mat4 Object::getModelMatrix() const {
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);
    return model;
}

void Object::draw(Shader& shader) const {
    // Ensure the updated model matrix is passed to the shader
    shader.setMat4("model", getModelMatrix());
    if (texture) {
        texture->bind();
        shader.setInt("tex", 0);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
    glBindVertexArray(0);
}


void Object::setPosition(const glm::vec3& pos) {
    position = pos;
}

void Object::setScale(const glm::vec3& s) {
    scale = s;
}

void Object::setRotation(const glm::vec3& axis) {
    rotation = axis;
}
