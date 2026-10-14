#pragma once

#include <vector>
#include <string>
#include <memory>
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

// OWNERSHIP MODEL (Scene.h owns Object lifetime overall):
//  - Object exclusively owns its GPU buffers (VAO/VBO); they are released in
//    the destructor, so there is no cleanup step to forget.
//  - `texture` is a NON-owning pointer used for rendering. It may point at a
//    shared texture (e.g. Interface's default texture) or at ownedTexture_.
//  - `parent` / `children` are non-owning references into whatever owns the
//    Objects (Scene). Scene::removeObject detaches an Object from the
//    hierarchy before destroying it.
//  - Copying gives the copy its OWN VAO/VBO (the old implicit copy duplicated
//    the raw handles, so deleting either object broke the other). A copy is
//    never parented and has no children.
class Object {
public:
    Object(std::string name, const std::vector<Vertex>& vertices, Texture* texture = nullptr);
    // Takes exclusive ownership of the texture.
    Object(std::string name, const std::vector<Vertex>& vertices, std::unique_ptr<Texture> ownedTexture);
    ~Object();

    Object(const Object& other);
    Object& operator=(const Object& other) = delete;
    Object(Object&& other) noexcept;
    Object& operator=(Object&& other) noexcept;

    std::string name = "Obj";
    std::vector<Vertex> vertices;
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
    glm::vec3 color;

    Texture* texture = nullptr; // non-owning; see class comment

    Object* parent = nullptr;
    std::vector<Object*> children;

    bool isSelected = false;
    bool showMeasureX = false;
    bool showMeasureY = false;
    bool showMeasureZ = false;

    // Game-engine properties (Roblox-style "Part" properties). Saved with the
    // scene; `velocity` and `visible` are also driven at runtime by physics
    // and scripts in play mode (GameRuntime restores them on Stop).
    struct GameProps {
        bool anchored = true;      // false: affected by gravity / pushed out of other parts
        bool canCollide = true;    // false: other parts pass through it (touch events still fire)
        bool visible = true;
        float bounciness = 0.0f;   // 0 = no bounce, 1 = perfectly elastic
        std::string script;        // RiftScript source, empty = no script
        glm::vec3 velocity{ 0.0f };
    } game;

    // Draws with the GLOBAL transform (own transform composed with every
    // ancestor's). The original used the local-only getModelMatrix(), so
    // parenting never had any visible effect.
    void draw(Shader& shader, Camera& camera) const;

    void setName(const std::string& newName);
    void setPosition(const glm::vec3& pos);
    void setScale(const glm::vec3& scale);
    void setRotation(const glm::vec3& axis);
    void setColor(const glm::vec3& newColor);

    // Borrowed texture: caller keeps ownership and must outlive its use here.
    void setTexture(Texture* newTexture);
    // Owned texture: freed automatically when replaced or when this Object dies.
    void setOwnedTexture(std::unique_ptr<Texture> newTexture);
    void removeTexture();

    void setParent(Object* newParent);
    void removeParent();
    void addChild(Object* child);
    void removeChild(Object* child);
    glm::mat4 getLocalTransform() const;
    glm::mat4 getGlobalTransform() const;
    bool isChildOf(Object* potentialParent) const;

    void setVertices(const std::vector<Vertex>& newVertices);

    // Axis-aligned bounds of the mesh in LOCAL space (zero if no vertices).
    // Picking and measurement lines use these, so they follow the real
    // shape, rotation and parent chain instead of a sphere around `position`.
    const glm::vec3& getBoundsMin() const { return boundsMin_; }
    const glm::vec3& getBoundsMax() const { return boundsMax_; }
    static std::vector<Vertex> fromJSONToVertices(const nlohmann::json& jsonVertices);

    // Purely local matrix (identical to getLocalTransform); kept for callers
    // that compose with the parent chain themselves.
    glm::mat4 getModelMatrix() const;

    nlohmann::json toJSON() const;
    void fromJSON(const nlohmann::json& j);

private:
    GLuint VAO = 0, VBO = 0;
    size_t vertexCount = 0;

    // Non-null only when this Object exclusively owns its texture; in that
    // case `texture` always equals ownedTexture_.get().
    std::unique_ptr<Texture> ownedTexture_;

    glm::vec3 boundsMin_{0.0f}, boundsMax_{0.0f};
    void computeBounds();
    void setup(const std::vector<Vertex>& vertices);
    void configureVertexAttributes() const;
    void releaseGPUResources();
};
