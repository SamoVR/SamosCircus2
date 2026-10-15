#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in float vPosY;

uniform sampler2D texture1;
uniform vec3 color;
uniform int hasTexture;  // New uniform

void main() {
    if (hasTexture == 1) {
        vec4 texColor = texture(texture1, TexCoord);

        float gradient = clamp((vPosY + 2.0) / 4.0, 0.0, 1.0);
        vec3 bgGradient = mix(vec3(0.05, 0.05, 0.1), vec3(0.1, 0.1, 0.3), gradient);

        FragColor = mix(vec4(bgGradient, 1.0), texColor, texColor.a);
    } else {
        FragColor = vec4(color, 1.0);  // Use color when no texture
    }
}
