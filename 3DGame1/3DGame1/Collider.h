#pragma once

#include <glm/glm.hpp>

class Collider {
public:
    glm::vec3 min; // Minimum corner of AABB
    glm::vec3 max; // Maximum corner of AABB

    Collider(const glm::vec3& position, const glm::vec3& size) {
        min = position - size * 0.5f;
        max = position + size * 0.5f;
    }

    bool intersects(const Collider& other) const {
        return (min.x <= other.max.x && max.x >= other.min.x) &&
            (min.y <= other.max.y && max.y >= other.min.y) &&
            (min.z <= other.max.z && max.z >= other.min.z);
    }
};
