#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in float vPosY;  // Receiving the Y position from the vertex shader

uniform sampler2D texture1;

void main() {
    vec4 texColor = texture(texture1, TexCoord);

    // Simple vertical gradient mix (adjust as needed)
    float gradient = clamp((vPosY + 2.0) / 4.0, 0.0, 1.0);  // You can tweak this formula
    vec3 bgGradient = mix(vec3(0.05, 0.05, 0.1), vec3(0.1, 0.1, 0.3), gradient);

    // Mix the background gradient with the texture color
    FragColor = mix(vec4(bgGradient, 1.0), texColor, texColor.a);
}
