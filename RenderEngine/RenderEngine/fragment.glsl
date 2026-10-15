#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
in float vPosY;
in vec3 WorldNormal;
in vec3 WorldPos;

uniform sampler2D texture1;
uniform vec3 color;
uniform int hasTexture;

// Single fixed "sun": direction the light travels (from light toward scene).
const vec3 kLightDirection = normalize(vec3(-0.4, -1.0, -0.3));
const vec3 kLightColor = vec3(1.0, 0.98, 0.92);
const float kAmbientStrength = 0.35;
const float kDiffuseStrength = 0.65;

void main() {
    // A zero-length normal means "no lighting information" (screen-space
    // quad, or vertex data saved before normals existed): skip lighting and
    // keep the original flat look instead of normalising a zero vector.
    float normalLength = length(WorldNormal);
    vec3 lighting = vec3(1.0);

    if (normalLength > 0.0001) {
        vec3 n = WorldNormal / normalLength;
        float diffuseFactor = max(dot(n, -kLightDirection), 0.0);
        lighting = (kAmbientStrength + kDiffuseStrength * diffuseFactor) * kLightColor;
    }

    if (hasTexture == 1) {
        vec4 texColor = texture(texture1, TexCoord);
        float gradient = clamp((vPosY + 2.0) / 4.0, 0.0, 1.0);
        vec3 bgGradient = mix(vec3(0.05, 0.05, 0.1), vec3(0.1, 0.1, 0.3), gradient);
        vec3 blended = mix(bgGradient, texColor.rgb, texColor.a);
        FragColor = vec4(blended * lighting, 1.0);
    } else {
        FragColor = vec4(color * lighting, 1.0);
    }
}
