#include "Renderer.h"

#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>

Renderer::Renderer() : shaderProgram(0), cubeVAO(0), cubeVBO(0) {

}

Renderer::~Renderer() {
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
    }
}

// Function to load a shader file as a string
std::string loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the entire file into the stringstream
    return buffer.str();    // Convert stringstream to string
}

GLuint Renderer::compileShader(const std::string& vertexPath, const std::string& fragmentPath) {
    // Load shader sources from files
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);

    const char* vertexShaderSource = vertexCode.c_str();
    const char* fragmentShaderSource = fragmentCode.c_str();

    // Create vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Create fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Link shaders to a program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check linking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Cleanup
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}


void Renderer::setShaderProgram(GLuint program) {
    shaderProgram = program;
    glUseProgram(shaderProgram);
}

void Renderer::setUniformMat4(const std::string& name, const glm::mat4& mat) {
    GLint location = glGetUniformLocation(shaderProgram, name.c_str());
    if (location == -1) {
        std::cerr << "Uniform '" << name << "' not found in shader program\n";
    }
    else {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
}

void Renderer::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::renderChunks(World& world) {
    glUseProgram(shaderProgram);

    for (auto& chunkPair : world.chunks) {
        Chunk* chunk = chunkPair.second;
        if (!chunk) continue;

        int chunkX = chunkPair.first.first;
        int chunkZ = chunkPair.first.second;

        for (int x = 0; x < CHUNK_SIZE; x++) {
            for (int y = 0; y < CHUNK_SIZE; y++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    Block* block = chunk->blocks[x][y][z]; // Get the block pointer
                    if (!block || block->type == BlockType::AIR) continue; // Skip air blocks

                    // Calculate the world position of the block
                    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(
                        static_cast<float>(chunkX * CHUNK_SIZE + x),
                        static_cast<float>(y),
                        static_cast<float>(chunkZ * CHUNK_SIZE + z)
                    ));

                    block->render(model, shaderProgram); // Use the existing block
                }
            }
        }
    }

    glBindVertexArray(0);
}
