#ifndef RENDERER_H
#define RENDERER_H

#include "World.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <fstream>
#include <string>

class Renderer {
public:
    Renderer();
    ~Renderer();

    GLuint compileShader(const std::string& vertexPath, const std::string& fragmentPath);
    void setShaderProgram(GLuint program);
    void setUniformMat4(const std::string& name, const glm::mat4& mat);
    void clear(float r, float g, float b, float a);

    void renderChunks(World& world);

private:
    GLuint shaderProgram;
    GLuint cubeVAO, cubeVBO;

};

#endif
