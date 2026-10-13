#include "Texture.h"
#include "Logger.h"
#include <algorithm>
#include <fstream>
#include <utility>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace {
    // The original only distinguished 4 channels (RGBA) from "everything
    // else" (RGB), so a 1- or 2-channel image was uploaded as RGB and read
    // past its pixel data. All four stb outputs are mapped explicitly now.
    GLenum formatForChannels(int channels) {
        switch (channels) {
            case 1: return GL_RED;
            case 2: return GL_RG;
            case 3: return GL_RGB;
            case 4: return GL_RGBA;
            default: return GL_RGB;
        }
    }
}

std::string Texture::normalizePath(const std::string& path) {
    std::string p = path;
    std::replace(p.begin(), p.end(), '\\', '/');
    if (p.compare(0, 7, "assets/") == 0) return p;
    const size_t pos = p.rfind("/assets/");
    if (pos != std::string::npos) return p.substr(pos + 1); // keep "assets/..."
    return p;
}

Texture::Texture(const std::string& path) {
    // Scene files used to store absolute paths like C:\Users\<name>\...\assets\images\x.png, so a
    // saved scene could not find its textures on any other machine. The stored path is now relative
    // to the assets folder, and old files are rescued: if the absolute path does not exist here,
    // the same file is looked up under ./assets.
    const std::string portable = normalizePath(path);
    std::string loadPath = path;
    {
        std::ifstream direct(path, std::ios::binary);
        if (!direct.good()) loadPath = portable;
    }
    filePath = portable;

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(loadPath.c_str(), &width, &height, &nrChannels, 0);

    if (!data) {
        // textureID stays 0 -> isValid() is false and the destructor won't
        // delete a texture that was never created.
        LOG_ERROR("Texture", "Failed to load image: " + path + " (" + stbi_failure_reason() + ")");
        return;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Rows of 1/2/3-channel images are not 4-byte aligned; GL's default
    // alignment of 4 would corrupt the upload for odd widths.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const GLenum format = formatForChannels(nrChannels);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
}

Texture::~Texture() { release(); }

void Texture::release() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}

Texture::Texture(Texture&& other) noexcept
    : textureID(other.textureID), filePath(std::move(other.filePath)) {
    other.textureID = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        release();
        textureID = other.textureID;
        filePath = std::move(other.filePath);
        other.textureID = 0;
    }
    return *this;
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureID);
}

const std::string& Texture::getFilePath() const { return filePath; }
