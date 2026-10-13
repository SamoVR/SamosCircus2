#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
// Inverse-transpose of the model matrix's 3x3 part, computed once per draw
// call on the CPU (see Object::draw). Using the model matrix directly would
// skew normals under non-uniform scale.
uniform mat3 normalMatrix;

out vec2 TexCoord;
out float vPosY;
out vec3 WorldNormal;
out vec3 WorldPos;

void main() {
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;
    TexCoord = aTexCoord;
    vPosY = aPos.y;
    WorldNormal = normalMatrix * aNormal; // normalised in the fragment shader
    WorldPos = worldPos.xyz;
}
