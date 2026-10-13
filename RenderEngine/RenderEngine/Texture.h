#pragma once

#include <string>
#include <GL/glew.h>

// Owns one OpenGL texture. Move-only: copying a GL handle would leave two
// C++ objects each believing they own (and will delete) the same resource.
class Texture {
public:
    explicit Texture(const std::string& path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    void bind(unsigned int unit = 0) const;
    // The path as it should be stored in scene files: relative to the project's
    // "assets" folder when the image lives there (portable between machines),
    // otherwise the normalised path as given.
    const std::string& getFilePath() const;

    // "C:\\Users\\me\\proj\\assets\\images\\a.png" -> "assets/images/a.png". Paths that
    // are not inside an assets folder are returned with forward slashes only.
    static std::string normalizePath(const std::string& path);

    // False if the image failed to load. Callers should check this instead
    // of binding a texture that silently contains nothing.
    bool isValid() const { return textureID != 0; }

private:
    void release();

    GLuint textureID = 0;
    std::string filePath;
};
