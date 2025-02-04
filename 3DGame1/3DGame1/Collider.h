#include <glm/glm.hpp>
#include <iostream>

class Collider {
public:
    // Constructor
    Collider(const glm::vec3& min, const glm::vec3& max)
        : min(min), max(max) {}

    // Check if two colliders are intersecting
    bool checkCollision(const Collider& other) const {
        // Check if there is no intersection on any axis
        if (max.x < other.min.x || min.x > other.max.x) return false; // X axis
        if (max.y < other.min.y || min.y > other.max.y) return false; // Y axis
        if (max.z < other.min.z || min.z > other.max.z) return false; // Z axis

        return true; // There is a collision if none of the above conditions are true
    }

    // Translate the collider
    void translate(const glm::vec3& translation) {
        min += translation;
        max += translation;
    }

    // Getters for the min and max corners
    glm::vec3 getMin() const { return min; }
    glm::vec3 getMax() const { return max; }

private:
    glm::vec3 min; // The minimum corner of the collider (e.g., lower-left-back)
    glm::vec3 max; // The maximum corner of the collider (e.g., upper-right-front)
};
