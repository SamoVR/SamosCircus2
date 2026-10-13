#pragma once
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position{0.0f};
    glm::vec2 texCoord{0.0f};
    // Zero normal == "no lighting information" (screen-space quad, or old
    // save files); fragment.glsl skips lighting for zero-length normals.
    glm::vec3 normal{0.0f};
};
