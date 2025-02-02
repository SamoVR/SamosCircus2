#ifndef BLOCK_H
#define BLOCK_H

#include <glm/glm.hpp>
#include <GL/glew.h>

enum class BlockType {
    AIR,
    GRASS
};

class Block {
public:
    BlockType type;
    GLuint VAO, VBO;

    // Declare the static vertices array, but don't define it here
    static float vertices[180]; // Static so that it can be shared among all instances of Block

    Block(BlockType type = BlockType::AIR);  // Constructor to set block type
    ~Block();

    void setup();  // Set up the cube (block) geometry
    void render(const glm::mat4& modelMatrix,GLuint shaderProgram);  // Render the block at a specific position
};

#endif // BLOCK_H
