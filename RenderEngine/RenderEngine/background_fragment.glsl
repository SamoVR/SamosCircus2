#version 330 core

out vec4 FragColor;

in vec2 TexCoord;  // Texture coordinates (though we won't be using them)

void main() {
    // Calculate a vertical gradient based on the Y-coordinate
    float gradient = TexCoord.y;  // Use the Y-coordinate of the quad's texture coordinate for the gradient

    // Color mix based on gradient value (you can change these colors)
    vec3 bgColor = mix(vec3(0.05, 0.05, 0.1), vec3(0.1, 0.1, 0.3), gradient);  // Dark blue to light blue gradient

    FragColor = vec4(bgColor, 1.0);  // Set the background color with full alpha
}
