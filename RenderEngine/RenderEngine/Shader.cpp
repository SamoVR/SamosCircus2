#include "Shader.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty()) {
        LOG_ERROR("Shader", std::string("Shader source is empty (check paths): ")
            + vertexPath + " / " + fragmentPath);
        return; // valid_ stays false, ID stays 0
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    const bool vertexOk = checkCompileErrors(vertex, "VERTEX");

    unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    const bool fragmentOk = checkCompileErrors(fragment, "FRAGMENT");

    bool linkOk = false;
    if (vertexOk && fragmentOk) {
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        linkOk = checkCompileErrors(ID, "PROGRAM");
    } else {
        LOG_ERROR("Shader", std::string("Skipping link due to compile failure: ")
            + vertexPath + " / " + fragmentPath);
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    valid_ = vertexOk && fragmentOk && linkOk;

    if (!valid_ && ID != 0) {
        glDeleteProgram(ID);
        ID = 0;
    }
}

Shader::~Shader() {
    if (ID != 0) glDeleteProgram(ID);
}

void Shader::use() const {
    glUseProgram(ID); // ID 0 (invalid shader) is a defined no-op in GL
}

GLint Shader::getUniformLocation(const std::string& name) const {
    auto it = uniformLocationCache_.find(name);
    if (it != uniformLocationCache_.end()) return it->second;
    GLint location = glGetUniformLocation(ID, name.c_str());
    uniformLocationCache_[name] = location;
    return location;
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setMat3(const std::string& name, const glm::mat3& mat) const {
    glUniformMatrix3fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(getUniformLocation(name), 1, glm::value_ptr(value));
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(getUniformLocation(name), value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(getUniformLocation(name), value);
}

std::string Shader::loadShaderSource(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Shader", std::string("Failed to open shader file: ") + path);
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool Shader::checkCompileErrors(unsigned int shader, const std::string& type) {
    int success;
    char infoLog[1024];

    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            LOG_ERROR("Shader", "Compile error (" + type + "): " + infoLog);
            return false;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            LOG_ERROR("Shader", "Link error: " + std::string(infoLog));
            return false;
        }
    }
    return true;
}
