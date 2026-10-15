#pragma once

#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/glm.hpp>

class Shader {
public:
    unsigned int ID = 0;

    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    void use() const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setFloat(const std::string& name, float value) const;
    void setInt(const std::string& name, int value) const;

    // False if the source failed to load or compile/link failed. Previously
    // a failure was only printed to std::cerr and the program carried on
    // with a dead shader, so everything drawn with it silently vanished.
    bool isValid() const { return valid_; }

private:
    std::string loadShaderSource(const char* path);
    bool checkCompileErrors(unsigned int shader, const std::string& type); // true on success

    // glGetUniformLocation is a string-keyed driver call; the original did
    // it on every set*() call for every object every frame. Cache per name.
    mutable std::unordered_map<std::string, GLint> uniformLocationCache_;
    GLint getUniformLocation(const std::string& name) const;

    bool valid_ = false;
};
