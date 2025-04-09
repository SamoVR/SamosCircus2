#pragma once

#include <string>
#include <GL/glew.h>

class Texture {
public:
    Texture(const std::string& path);
    ~Texture();


    void bind(unsigned int unit = 0) const;

private:
    GLuint textureID;
};
