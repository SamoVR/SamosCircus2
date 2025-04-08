#version 330 core

layout (location = 0) in vec2 aPos;  // Position of the vertex
layout (location = 1) in vec2 aTexCoord;  // Texture coordinates (not used for the background, but still required for compatibility)

out vec2 TexCoord;  // Output the texture coordinates

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);  // Transform the vertex position
    TexCoord = aTexCoord;  // Pass texture coordinates (if needed in the future)
}
