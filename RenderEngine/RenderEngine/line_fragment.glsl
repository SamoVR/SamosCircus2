#version 330 core
out vec4 FragColor;

// Colour of the line being drawn (the measurement lines use red/green/blue per
// axis). This uniform was commented out and the shader hardcoded white, so the
// colour passed to Engine::drawLine was silently ignored.
uniform vec3 lineColor;

void main() {
    FragColor = vec4(lineColor, 1.0);
}
