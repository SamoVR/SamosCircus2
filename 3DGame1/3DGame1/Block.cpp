#include "Block.h"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint Block::textureAtlasID = 0;

// Cube vertices (Position + Texture Coords) for one block
float Block::vertices[] = {
    // Front face
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    // Back face
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    // Left face
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    // Right face
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

     // Bottom face
     -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
      0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
      0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

     // Top face
     -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
      0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

// Constructor
Block::Block(BlockType type, float breakTime, bool isSolid, std::array<int, 6> textureIDs)
    : type(type), breakTime(breakTime), isSolid(isSolid), textureIDs(textureIDs), VAO(0), VBO(0)
{
    setup();
}


Block::~Block() {
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

std::string blockTypeToString(BlockType type) {
    switch (type) {
    case BlockType::GRASS:
        return "GRASS";
    case BlockType::STONE:
        return "STONE";
    case BlockType::DIRT:
        return "DIRT";
    case BlockType::BEDROCK:
        return "BEDROCK";
        // Add other cases...
    default:
        return "UNKNOWN";
    }
}

void Block::setup() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    const int atlasSize = 16;
    float texSize = 1.0f;

    float adjustedVertices[180];

    for (int i = 0; i < 6; i++) {
        int texIndex = textureIDs[i];

        int texX = texIndex / atlasSize;
        int texY = texIndex % atlasSize;

        float uMin = texX * texSize;
        float vMin = texY * texSize;
        float uMax = uMin + texSize;
        float vMax = vMin + texSize;

        for (int j = 0; j < 6; j++) {
            int vertIndex = (i * 30) + (j * 5);
            adjustedVertices[vertIndex] = vertices[vertIndex];
            adjustedVertices[vertIndex + 1] = vertices[vertIndex + 1];
            adjustedVertices[vertIndex + 2] = vertices[vertIndex + 2];

            float originalU = vertices[vertIndex + 3];
            float originalV = vertices[vertIndex + 4];

            adjustedVertices[vertIndex + 3] = (originalU == 0.0f) ? uMin : uMax;
            adjustedVertices[vertIndex + 4] = (originalV == 0.0f) ? vMin : vMax;
        }
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(adjustedVertices), adjustedVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// Render the block
void Block::render(const glm::mat4& modelMatrix, GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlasID);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

}

// Load the texture atlas for the block
void Block::loadTextureAtlas(const std::string& filePath) {
    glGenTextures(1, &textureAtlasID);
    glBindTexture(GL_TEXTURE_2D, textureAtlasID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, numChannels;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &numChannels, 0);
    if (data) {
        GLenum format = (numChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Texture atlas loaded: " << filePath << std::endl;
    }
    else {
        std::cerr << "Failed to load texture atlas: " << filePath << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

BlockType Block::getType() {
    return type;
}
