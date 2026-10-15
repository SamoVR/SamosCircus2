#include "Object.h"
#include "Logger.h"
#include <algorithm>
#include <cstddef>

Object::Object(std::string name, const std::vector<Vertex>& vertices, Texture* texture)
    : name(std::move(name)),
    vertices(vertices),
    color(1.0f),
    texture(texture),
    vertexCount(vertices.size())
{
    setup(vertices);
}

Object::Object(std::string name, const std::vector<Vertex>& vertices, std::unique_ptr<Texture> ownedTexture)
    : name(std::move(name)),
    vertices(vertices),
    color(1.0f),
    texture(ownedTexture.get()),
    vertexCount(vertices.size()),
    ownedTexture_(std::move(ownedTexture))
{
    setup(vertices);
}

// Explicit copy: allocates fresh GPU buffers via setup() instead of copying
// the VAO/VBO handles (the old implicit copy shared them). The copy is not
// parented and has no children; an exclusively-owned texture is reloaded from
// its file so the copy owns an independent one, while a shared texture stays
// shared.
Object::Object(const Object& other)
    : name(other.name),
    vertices(other.vertices),
    position(other.position),
    rotation(other.rotation),
    scale(other.scale),
    color(other.color),
    texture(other.texture),
    isSelected(false),
    showMeasureX(other.showMeasureX),
    showMeasureY(other.showMeasureY),
    showMeasureZ(other.showMeasureZ),
    vertexCount(other.vertices.size())
{
    if (other.ownedTexture_) {
        ownedTexture_ = std::make_unique<Texture>(other.ownedTexture_->getFilePath());
        texture = ownedTexture_.get();
        if (!ownedTexture_->isValid()) {
            LOG_WARN("Object", "Copy of '" + other.name + "': failed to reload texture "
                + other.ownedTexture_->getFilePath());
        }
    }
    setup(vertices);
}

Object::Object(Object&& other) noexcept
    : name(std::move(other.name)),
    vertices(std::move(other.vertices)),
    position(other.position),
    rotation(other.rotation),
    scale(other.scale),
    color(other.color),
    texture(other.texture),
    parent(other.parent),
    children(std::move(other.children)),
    isSelected(other.isSelected),
    showMeasureX(other.showMeasureX),
    showMeasureY(other.showMeasureY),
    showMeasureZ(other.showMeasureZ),
    VAO(other.VAO),
    VBO(other.VBO),
    vertexCount(other.vertexCount),
    ownedTexture_(std::move(other.ownedTexture_))
{
    boundsMin_ = other.boundsMin_;
    boundsMax_ = other.boundsMax_;
    other.VAO = 0;
    other.VBO = 0;
    other.texture = nullptr;
    other.parent = nullptr;
}

Object& Object::operator=(Object&& other) noexcept {
    if (this != &other) {
        releaseGPUResources();

        name = std::move(other.name);
        vertices = std::move(other.vertices);
        position = other.position;
        rotation = other.rotation;
        scale = other.scale;
        color = other.color;
        texture = other.texture;
        parent = other.parent;
        children = std::move(other.children);
        isSelected = other.isSelected;
        showMeasureX = other.showMeasureX;
        showMeasureY = other.showMeasureY;
        showMeasureZ = other.showMeasureZ;
        VAO = other.VAO;
        VBO = other.VBO;
        vertexCount = other.vertexCount;
        ownedTexture_ = std::move(other.ownedTexture_);
        boundsMin_ = other.boundsMin_;
        boundsMax_ = other.boundsMax_;

        other.VAO = 0;
        other.VBO = 0;
        other.texture = nullptr;
        other.parent = nullptr;
    }
    return *this;
}

Object::~Object() {
    releaseGPUResources();
}

void Object::releaseGPUResources() {
    if (VAO != 0) glDeleteVertexArrays(1, &VAO);
    if (VBO != 0) glDeleteBuffers(1, &VBO);
    VAO = 0;
    VBO = 0;
}

// Shared by setup() and setVertices(): position (0), texCoord (1), normal (2).
void Object::configureVertexAttributes() const {
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
}

void Object::computeBounds() {
    if (vertices.empty()) { boundsMin_ = boundsMax_ = glm::vec3(0.0f); return; }
    boundsMin_ = boundsMax_ = vertices[0].position;
    for (const Vertex& v : vertices) {
        boundsMin_ = glm::min(boundsMin_, v.position);
        boundsMax_ = glm::max(boundsMax_, v.position);
    }
}

void Object::setup(const std::vector<Vertex>& verts) {
    computeBounds();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    configureVertexAttributes();
    glBindVertexArray(0);
}

glm::mat4 Object::getModelMatrix() const {
    return getLocalTransform();
}

void Object::draw(Shader& shader, Camera& camera) const {
    (void)camera; // kept for API compatibility

    // glUniform* calls apply to whichever program is bound. The engine interleaves other
    // shaders (measurement lines) between object draws, so bind ours explicitly.
    shader.use();

    const glm::mat4 modelMatrix = getGlobalTransform();
    shader.setMat4("model", modelMatrix);

    // Normals must use the inverse-transpose of the model matrix, otherwise
    // non-uniform scale (e.g. stretching one axis with the gizmo) tilts them.
    shader.setMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(modelMatrix))));

    if (texture && texture->isValid()) {
        texture->bind();
        shader.setInt("texture1", 0);
        shader.setInt("hasTexture", 1);
    }
    else {
        shader.setVec3("color", color);
        shader.setInt("hasTexture", 0);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
    glBindVertexArray(0);
}

void Object::setName(const std::string& newName) { name = newName; }
void Object::setPosition(const glm::vec3& pos) { position = pos; }
void Object::setScale(const glm::vec3& s) { scale = s; }
void Object::setRotation(const glm::vec3& axis) { rotation = axis; }

void Object::setColor(const glm::vec3& newColor) {
    color = newColor;
    texture = nullptr;     // color mode: no texture
    ownedTexture_.reset();
}

void Object::setTexture(Texture* newTexture) {
    ownedTexture_.reset(); // we're switching to a pointer someone else owns
    texture = newTexture;
}

void Object::setOwnedTexture(std::unique_ptr<Texture> newTexture) {
    ownedTexture_ = std::move(newTexture); // frees the previously owned one
    texture = ownedTexture_.get();
}

void Object::removeTexture() {
    texture = nullptr;
    ownedTexture_.reset();
}

void Object::setParent(Object* newParent) {
    if (newParent == this) {
        LOG_WARN("Object", "Refusing to parent '" + name + "' to itself.");
        return;
    }
    if (newParent && newParent->isChildOf(this)) {
        LOG_WARN("Object", "Refusing to parent '" + name + "' under its own descendant '" + newParent->name + "'.");
        return;
    }
    if (parent) {
        parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), this), parent->children.end());
    }
    parent = newParent;
    if (newParent) newParent->children.push_back(this);
}

void Object::removeParent() {
    if (parent) {
        parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), this), parent->children.end());
        parent = nullptr;
    }
}

void Object::addChild(Object* child) {
    if (!child) return;
    child->setParent(this); // detaches from any old parent and rejects cycles
}

void Object::removeChild(Object* child) {
    if (!child) return;
    children.erase(std::remove(children.begin(), children.end(), child), children.end());
    if (child->parent == this) child->parent = nullptr;
}

bool Object::isChildOf(Object* potentialParent) const {
    const Object* current = parent;
    while (current) {
        if (current == potentialParent) return true;
        current = current->parent;
    }
    return false;
}

nlohmann::json Object::toJSON() const {
    nlohmann::json j;
    j["name"] = name;
    j["position"] = { position.x, position.y, position.z };
    j["rotation"] = { rotation.x, rotation.y, rotation.z };
    j["scale"] = { scale.x, scale.y, scale.z };
    j["color"] = { color.x, color.y, color.z };
    if (parent) j["parent"] = parent->name;

    nlohmann::json verticesJson = nlohmann::json::array();
    for (const auto& vertex : vertices) {
        nlohmann::json vertexJson;
        vertexJson["position"] = { vertex.position.x, vertex.position.y, vertex.position.z };
        vertexJson["texCoord"] = { vertex.texCoord.x, vertex.texCoord.y };
        vertexJson["normal"] = { vertex.normal.x, vertex.normal.y, vertex.normal.z };
        verticesJson.push_back(vertexJson);
    }
    j["vertices"] = verticesJson;

    if (texture) {
        j["texture"] = texture->getFilePath();
        j["textureOwned"] = (ownedTexture_ != nullptr);
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
        const std::string texturePath = j["texture"];
        // Older save files have no "textureOwned" flag and always loaded a
        // per-object texture, so default to true.
        const bool wasOwned = j.value("textureOwned", true);
        if (wasOwned) {
            auto loaded = std::make_unique<Texture>(texturePath);
            if (!loaded->isValid()) {
                LOG_WARN("Object", "'" + name + "': failed to load texture from save file: " + texturePath);
            }
            setOwnedTexture(std::move(loaded));
        }
        // A shared (non-owned) texture is re-attached by the caller that
        // knows which shared Texture* to use (see Scene::loadFromFile).
    }

    if (j.contains("vertices")) {
        setVertices(fromJSONToVertices(j["vertices"]));
    }
}

std::vector<Vertex> Object::fromJSONToVertices(const nlohmann::json& jsonVertices) {
    std::vector<Vertex> result;
    result.reserve(jsonVertices.size());
    for (const auto& v : jsonVertices) {
        Vertex vertex{};
        vertex.position = glm::vec3(v["position"][0], v["position"][1], v["position"][2]);
        vertex.texCoord = glm::vec2(v["texCoord"][0], v["texCoord"][1]);
        // Normals are optional so scenes saved before they existed still load
        // (zero normal == unlit, see fragment.glsl).
        if (v.contains("normal")) {
            vertex.normal = glm::vec3(v["normal"][0], v["normal"][1], v["normal"][2]);
        }
        result.push_back(vertex);
    }
    return result;
}

void Object::setVertices(const std::vector<Vertex>& newVertices) {
    vertices = newVertices;
    vertexCount = newVertices.size();
    computeBounds();

    if (VBO != 0) glDeleteBuffers(1, &VBO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), newVertices.data(), GL_STATIC_DRAW);
    configureVertexAttributes();
    glBindVertexArray(0);
}

glm::mat4 Object::getLocalTransform() const {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);
    return model;
}

glm::mat4 Object::getGlobalTransform() const {
    if (parent) return parent->getGlobalTransform() * getLocalTransform();
    return getLocalTransform();
}
