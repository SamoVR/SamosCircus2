#include "Object.h"

Object::Object(std::string name, const std::vector<Vertex>& vertices, Texture* texture)
    : name(name), vertexCount(vertices.size()),vertices(vertices), texture(texture), position(0.0f), rotation(0.0f), scale(1.0f), color(1.0f)
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

void Object::draw(Shader& shader, Camera& camera) const {
    shader.setMat4("model", getModelMatrix());

    if (texture) {
        texture->bind();
        shader.setInt("texture1", 0);
        shader.setInt("hasTexture", 1); // Texture is present
    }
    else {
        shader.setVec3("color", color);
        shader.setInt("hasTexture", 0); // No texture, use color
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

void Object::setColor(const glm::vec3& newColor) {
    color = newColor;
    texture = nullptr;  // Remove texture if color is set
}

void Object::setTexture(Texture* newTexture) {
    texture = newTexture;
}

void Object::removeTexture() {
    texture = nullptr;
}

nlohmann::json Object::toJSON() const {
    nlohmann::json j;
    j["name"] = name;
    j["position"] = { position.x, position.y, position.z };
    j["rotation"] = { rotation.x, rotation.y, rotation.z };
    j["scale"] = { scale.x, scale.y, scale.z };
    j["color"] = { color.x, color.y, color.z };

    // Serialize vertices
    nlohmann::json verticesJson = nlohmann::json::array();
    for (const auto& vertex : vertices) {
        nlohmann::json vertexJson;
        vertexJson["position"] = { vertex.position.x, vertex.position.y, vertex.position.z };
        vertexJson["texCoord"] = { vertex.texCoord.x, vertex.texCoord.y };
        verticesJson.push_back(vertexJson);
    }
    j["vertices"] = verticesJson;

    if (texture) {
        j["texture"] = texture->getFilePath();
    }

    return j;
}


void Object::fromJSON(const nlohmann::json& j) {
    if (j.contains("name")) name = j["name"];
    if (j.contains("position")) position = glm::vec3(j["position"][0], j["position"][1], j["position"][2]);
    if (j.contains("rotation")) rotation = glm::vec3(j["rotation"][0], j["rotation"][1], j["rotation"][2]);
    if (j.contains("scale")) scale = glm::vec3(j["scale"][0], j["scale"][1], j["scale"][2]);
    if (j.contains("color")) color = glm::vec3(j["color"][0], j["color"][1], j["color"][2]);

    if (j.contains("texture")) {
        std::string texturePath = j["texture"];
        texture = new Texture(texturePath);  // Load texture from file path
    }

    // If vertices are provided in the JSON, load them
    if (j.contains("vertices")) {
        std::vector<Vertex> vertices = fromJSONToVertices(j["vertices"]);
        setVertices(vertices);  // Set the vertices using the new method
    }
}


std::vector<Vertex> Object::fromJSONToVertices(const nlohmann::json& jsonVertices) {
    std::vector<Vertex> vertices;
    for (const auto& v : jsonVertices) {
        Vertex vertex;
        vertex.position = glm::vec3(v["position"][0], v["position"][1], v["position"][2]);
        vertex.texCoord = glm::vec2(v["texCoord"][0], v["texCoord"][1]);
        vertices.push_back(vertex);
    }
    return vertices;
}


void Object::setVertices(const std::vector<Vertex>& newVertices) {
    vertexCount = newVertices.size();

    // Delete the old buffers
    glDeleteBuffers(1, &VBO);

    // Generate a new buffer and upload the new vertex data
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), newVertices.data(), GL_STATIC_DRAW);

    // Re-setup the vertex attributes
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);
}

