#ifndef BLOCK_H
#define BLOCK_H

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

    static GLuint textureAtlasID;  // Holds the Minecraft texture atlas
    static float vertices[180];

    Block(BlockType type, float breakTime, bool isSolid, std::array<int, 6> textureIDs);
    virtual ~Block();

    void setup();
    virtual void render(const glm::mat4& modelMatrix, GLuint shaderProgram);

    static void loadTextureAtlas(const std::string& filePath);

    BlockType getType();
};

#endif // BLOCK_H
