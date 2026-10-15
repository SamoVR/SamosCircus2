#version 330 core

layout (location = 0) in vec3 aPos;       // Vertex position
layout (location = 1) in vec2 aTexCoord;  // Texture coordinates

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoord;  // Pass texture coords
out float vPosY;    // Pass Y position for gradient calculation

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    vPosY = aPos.y;  // Send the Y position to the fragment shader
}
