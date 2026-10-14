#include "Object.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include "Geometry.h"

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
    game(other.game),
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
    game(other.game),
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
        game = other.game;
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
    if (!game.visible) return;

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

namespace {
    // 5 decimals is far below anything visible and keeps the file short
    // ("-0.0611194372177124" -> "-0.06112").
    double round5(float v) {
        const double r = std::round(static_cast<double>(v) * 1e5) / 1e5;
        return r == 0.0 ? 0.0 : r; // no "-0.0"
    }
    nlohmann::json vec3Json(const glm::vec3& v) { return { round5(v.x), round5(v.y), round5(v.z) }; }
    glm::vec3 jsonVec3(const nlohmann::json& j) { return { j.at(0).get<float>(), j.at(1).get<float>(), j.at(2).get<float>() }; }
}

// Scene format v2: only non-default values are written, and a built-in
// primitive is stored as its name ("mesh": "sphere") instead of thousands of
// vertices. Other meshes go in "v" as one flat array (8 numbers per vertex:
// position, uv, normal). Files in the old format still load.
nlohmann::json Object::toJSON() const {
    nlohmann::json j;
    j["name"] = name;
    j["position"] = vec3Json(position);
    if (rotation != glm::vec3(0.0f)) j["rotation"] = vec3Json(rotation);
    if (scale != glm::vec3(1.0f)) j["scale"] = vec3Json(scale);
    if (color != glm::vec3(1.0f)) j["color"] = vec3Json(color);
    if (parent) j["parent"] = parent->name;

    const std::string primitive = identifyPrimitive(vertices);
    if (!primitive.empty()) {
        j["mesh"] = primitive;
    } else if (!vertices.empty()) {
        nlohmann::json flat = nlohmann::json::array();
        for (const auto& v : vertices) {
            for (float f : { v.position.x, v.position.y, v.position.z, v.texCoord.x, v.texCoord.y, v.normal.x, v.normal.y, v.normal.z }) {
                flat.push_back(round5(f));
            }
        }
        j["mesh"] = "custom";
        j["v"] = std::move(flat);
    }

    if (texture) {
        j["texture"] = texture->getFilePath();
        j["textureOwned"] = ownedTexture_ != nullptr;
    }

    const GameProps defaults;
    nlohmann::json g = nlohmann::json::object();
    if (game.anchored != defaults.anchored) g["anchored"] = game.anchored;
    if (game.canCollide != defaults.canCollide) g["canCollide"] = game.canCollide;
    if (game.visible != defaults.visible) g["visible"] = game.visible;
    if (game.bounciness != defaults.bounciness) g["bounciness"] = round5(game.bounciness);
    if (!game.script.empty()) g["script"] = game.script;
    if (!g.empty()) j["game"] = std::move(g);
    return j;
}

void Object::fromJSON(const nlohmann::json& j) {
    if (j.contains("name")) name = j["name"];
    if (j.contains("position")) position = jsonVec3(j["position"]);
    if (j.contains("rotation")) rotation = jsonVec3(j["rotation"]);
    if (j.contains("scale")) scale = jsonVec3(j["scale"]);
    if (j.contains("color")) color = jsonVec3(j["color"]);

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

    // Scenes saved before the game properties existed get the defaults (static, collidable parts).
    if (j.contains("game") && j["game"].is_object()) {
        const auto& g = j["game"];
        game.anchored = g.value("anchored", true);
        game.canCollide = g.value("canCollide", true);
        game.visible = g.value("visible", true);
        game.bounciness = g.value("bounciness", 0.0f);
        game.script = g.value("script", std::string());
    }

    const std::string mesh = j.value("mesh", std::string());
    if (mesh == "custom" && j.contains("v")) {
        const auto& flat = j["v"];
        std::vector<Vertex> verts(flat.size() / 8);
        for (size_t i = 0; i < verts.size(); ++i) {
            const size_t k = i * 8;
            verts[i].position = { flat[k].get<float>(), flat[k + 1].get<float>(), flat[k + 2].get<float>() };
            verts[i].texCoord = { flat[k + 3].get<float>(), flat[k + 4].get<float>() };
            verts[i].normal = { flat[k + 5].get<float>(), flat[k + 6].get<float>(), flat[k + 7].get<float>() };
        }
        setVertices(verts);
    } else if (!mesh.empty()) {
        const std::vector<Vertex> verts = primitiveVertices(mesh);
        if (verts.empty()) LOG_WARN("Object", "'" + name + "': unknown mesh '" + mesh + "'");
        setVertices(verts);
    } else if (j.contains("vertices")) { // scene format v1
        std::vector<Vertex> verts = fromJSONToVertices(j["vertices"]);
        if (verts.empty()) { // old builds sometimes saved primitives with no vertices (invisible objects)
            LOG_WARN("Object", "'" + name + "': empty mesh in scene file, using a cube");
            verts = createCubeVertices();
        }
        // v1 files store primitives vertex by vertex, some from before normals
        // existed (unlit). Swap in the current primitive so they get lighting.
        const std::string primitive = identifyPrimitive(verts);
        if (!primitive.empty()) verts = primitiveVertices(primitive);
        setVertices(verts);
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
