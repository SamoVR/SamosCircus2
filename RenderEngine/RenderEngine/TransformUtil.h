#pragma once
//
// TransformUtil.h -- matrix <-> (position, rotation, scale) conversion that
// matches the engine's own convention exactly:
//
//     M = T(position) * Rx(rot.x) * Ry(rot.y) * Rz(rot.z) * S(scale)
//
// (see Object::getLocalTransform). The gizmo code used to rebuild Euler angles
// with hand-written atan2 formulas that do not invert that composition, so
// rotating with the gizmo could produce wrong or unstable angles.
//
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

namespace TransformUtil {

inline glm::mat4 compose(const glm::vec3& position, const glm::vec3& rotationDeg, const glm::vec3& scale) {
    glm::mat4 m = glm::translate(glm::mat4(1.0f), position);
    m = glm::rotate(m, glm::radians(rotationDeg.x), glm::vec3(1, 0, 0));
    m = glm::rotate(m, glm::radians(rotationDeg.y), glm::vec3(0, 1, 0));
    m = glm::rotate(m, glm::radians(rotationDeg.z), glm::vec3(0, 0, 1));
    return glm::scale(m, scale);
}

// Inverse of compose(). Shear is discarded. A mirrored matrix is represented
// as a negative X scale. Angles are in degrees; at gimbal lock (|rot.y| = 90)
// the rotation about Z is reported as 0.
inline void decompose(const glm::mat4& m, glm::vec3& position, glm::vec3& rotationDeg, glm::vec3& scale) {
    position = glm::vec3(m[3]);

    glm::vec3 axes[3] = { glm::vec3(m[0]), glm::vec3(m[1]), glm::vec3(m[2]) };
    scale = glm::vec3(glm::length(axes[0]), glm::length(axes[1]), glm::length(axes[2]));

    glm::mat3 R(1.0f);
    for (int i = 0; i < 3; ++i) {
        if (scale[i] > 1e-8f) R[i] = axes[i] / scale[i];
    }
    if (glm::determinant(R) < 0.0f) { // mirrored
        scale.x = -scale.x;
        R[0] = -R[0];
    }

    // R = Rx * Ry * Rz  =>  R(row0,col2) = sin(y). GLM is column-major: R[col][row].
    const float sy = glm::clamp(R[2][0], -1.0f, 1.0f);
    float x, y = std::asin(sy), z;
    if (std::abs(sy) < 0.99999f) {
        x = std::atan2(-R[2][1], R[2][2]);
        z = std::atan2(-R[1][0], R[0][0]);
    } else { // gimbal lock: x and z are coupled, fold everything into x
        x = (sy > 0.0f) ? std::atan2(R[0][1], R[1][1]) : -std::atan2(R[0][1], R[1][1]);
        z = 0.0f;
    }
    rotationDeg = glm::vec3(glm::degrees(x), glm::degrees(y), glm::degrees(z));
}

} // namespace TransformUtil
