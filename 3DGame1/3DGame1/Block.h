#ifndef BLOCK_H
#define BLOCK_H

#include "Collider.h"  // Include the Collider header
#include "ColliderVisualizer.h"

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <array>
#include <string>

enum class BlockType {
    AIR,
    GRASS,
    DIRT,
    STONE,
    BEDROCK
};

class Block {
public:
    BlockType type;
    GLuint VAO, VBO;
    float breakTime;
    bool isSolid;
    std::array<int, 6> textureIDs; // Six different textures (one per face)

    // Collider object to represent the block's collision area
    Collider collider;

    static GLuint textureAtlasID;  // Holds the Minecraft texture atlas
    static float vertices[180];

    Block(BlockType type, float breakTime, bool isSolid, std::array<int, 6> textureIDs);
    virtual ~Block();

    void setup();
    virtual void render(const glm::mat4& modelMatrix, GLuint shaderProgram);

    void renderCollider(const glm::mat4& modelMatrix, GLuint shaderProgram);

    static void loadTextureAtlas(const std::string& filePath);

    BlockType getType();

    // Additional function to access the block's collider
    Collider getCollider() const { return collider; }

    // Helper method to translate the collider's position
    void updateColliderPosition(const glm::vec3& position);
};

#endif // BLOCK_H
