#include "Geometry.h"

#include <cmath>

namespace glm {
    const float pi = 3.14159265358979323846f;  // Define pi manually
    const float two_pi = 6.28318530717958647692f; // Define two_pi manually
}

std::vector<Vertex> createCubeVertices() {
    return {
        // Front face
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},

        // Back face
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},

        // Left face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        {{-0.5f,  0.5f,  0.5f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},

        // Right face
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f}},

        // Top face
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f}},

        // Bottom face
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f}},
        {{-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f}},
    };
}

std::vector<Vertex> createTriangle3DVertices() {
    return {
        // Base
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 0.0f}},
        {{ 0.5f, 0.0f, -0.5f}, {1.0f, 0.0f}},
        {{ 0.0f, 0.0f,  0.5f}, {0.5f, 1.0f}},

        // Side 1
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 0.0f}},
        {{ 0.0f, 0.8f,  0.0f}, {0.5f, 1.0f}},
        {{ 0.5f, 0.0f, -0.5f}, {1.0f, 0.0f}},

        // Side 2
        {{ 0.5f, 0.0f, -0.5f}, {1.0f, 0.0f}},
        {{ 0.0f, 0.8f,  0.0f}, {0.5f, 1.0f}},
        {{ 0.0f, 0.0f,  0.5f}, {0.0f, 0.0f}},

        // Side 3
        {{ 0.0f, 0.0f,  0.5f}, {0.0f, 0.0f}},
        {{ 0.0f, 0.8f,  0.0f}, {0.5f, 1.0f}},
        {{-0.5f, 0.0f, -0.5f}, {1.0f, 0.0f}},
    };
}

std::vector<Vertex> createSphereVertices(float radius, int stacks, int slices) {
    std::vector<Vertex> vertices;

    // Generate vertices for the sphere
    for (int i = 0; i <= stacks; ++i) {
        float V = float(i) / float(stacks);
        float phi = glm::pi * V;  // Use glm::pi manually

        for (int j = 0; j <= slices; ++j) {
            float U = float(j) / float(slices);
            float theta = glm::two_pi * U;  // Use glm::two_pi manually

            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            glm::vec2 uv = { U, V };
            vertices.push_back({ {x, y, z}, uv });
        }
    }

    std::vector<Vertex> sphere;

    // Generate indices for the sphere faces
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            // Create two triangles per quad
            sphere.push_back(vertices[first]);
            sphere.push_back(vertices[second]);
            sphere.push_back(vertices[first + 1]);

            sphere.push_back(vertices[second]);
            sphere.push_back(vertices[second + 1]);
            sphere.push_back(vertices[first + 1]);
        }
    }

    return sphere;
}

std::vector<Vertex> createFullScreenQuadVertices() {
    return {
        // Positions           // Texture Coordinates
        {{-1.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},

        {{-1.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f,  0.0f}, {0.0f, 1.0f}}
    };
}
