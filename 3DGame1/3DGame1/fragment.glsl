#version 330 core
in vec2 TexCoord;
flat in int TexIndex;

out vec4 FragColor;

uniform sampler2D texture1;

void main() {
    const float TILE_SIZE = 1.0 / 16.0;  // Minecraft atlas is 16x16
    float xOffset = (TexIndex % 16) * TILE_SIZE;
    float yOffset = (TexIndex / 16) * TILE_SIZE;
    
    vec2 atlasUV = TexCoord * TILE_SIZE + vec2(xOffset, yOffset);
    FragColor = texture(texture1, atlasUV);
}
