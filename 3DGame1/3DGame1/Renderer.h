#ifndef RENDERER_H
#define RENDERER_H

#include "Shader.h"
#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Renderer {
public:
    Renderer();
    void render(const Camera& camera);
    ~Renderer();
private:
    GLuint VAO, VBO;
    Shader shader;
    void setupRoom();
};

#endif
