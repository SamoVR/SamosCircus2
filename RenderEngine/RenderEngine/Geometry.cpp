#include "Geometry.h"

#include <cmath>

namespace glm {
    const float pi = 3.14159265358979323846f;
    const float two_pi = 6.28318530717958647692f;
}

namespace {
    // Flat-shaded face normal from triangle winding (counter-clockwise when
    // viewed from outside, the winding already used by every primitive here).
    // Computing it from positions keeps normals correct-by-construction: no
    // ~48 hand-typed normals that could drift from the positions on edit.
    void computeFlatNormals(std::vector<Vertex>& vertices) {
        for (size_t i = 0; i + 2 < vertices.size(); i += 3) {
            const glm::vec3 edge1 = vertices[i + 1].position - vertices[i].position;
            const glm::vec3 edge2 = vertices[i + 2].position - vertices[i].position;
            const glm::vec3 n = glm::normalize(glm::cross(edge1, edge2));
            vertices[i].normal = n;
            vertices[i + 1].normal = n;
            vertices[i + 2].normal = n;
        }
    }
}

std::vector<Vertex> createCubeVertices() {
    std::vector<Vertex> cube = {
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
    computeFlatNormals(cube);
    return cube;
}

std::vector<Vertex> createTriangle3DVertices() {
    std::vector<Vertex> pyramid = {
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
    computeFlatNormals(pyramid);
    return pyramid;
}

std::vector<Vertex> createSphereVertices(float radius, int stacks, int slices) {
    std::vector<Vertex> vertices;

    for (int i = 0; i <= stacks; ++i) {
        float V = float(i) / float(stacks);
        float phi = glm::pi * V;

        for (int j = 0; j <= slices; ++j) {
            float U = float(j) / float(slices);
            float theta = glm::two_pi * U;

            glm::vec3 position = {
                radius * sin(phi) * cos(theta),
                radius * cos(phi),
                radius * sin(phi) * sin(theta)
            };

            Vertex v;
            v.position = position;
            v.texCoord = { U, V };
            // For a sphere centred on the origin the outward normal is just
            // the normalised position: exact, and smooth-shaded (per-face
            // normals would make the sphere look faceted).
            v.normal = glm::normalize(position);
            vertices.push_back(v);
        }
    }

    std::vector<Vertex> sphere;
    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < slices; ++j) {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

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
    // Screen-space quad: deliberately no normals (zero == unlit).
    return {
        {{-1.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f,  0.0f}, {1.0f, 0.0f}},
        {{ 1.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},

        {{-1.0f, -1.0f,  0.0f}, {0.0f, 0.0f}},
        {{ 1.0f,  1.0f,  0.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f,  0.0f}, {0.0f, 1.0f}}
    };
}
