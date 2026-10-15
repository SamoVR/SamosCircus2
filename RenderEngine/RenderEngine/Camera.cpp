#include "Camera.h"
#include <algorithm>
#include <cmath>

Camera::Camera(float aspectRatio)
    : target(0.0f), position(0.0f, 0.0f, 5.0f),
    distance(5.0f), yaw(0.0f), pitch(0.0f),
    targetDistance(5.0f), targetYaw(0.0f), targetPitch(0.0f), targetPanPoint(0.0f),
    zoomSpeed(1.0f), rotateSpeed(0.3f), panSpeed(0.005f),
    fov(glm::radians(45.0f)), aspect(aspectRatio), nearPlane(0.1f), farPlane(100.0f) {
}

void Camera::handleMouseInput(float deltaX, float deltaY, bool rotating, bool panning) {
    // Input only moves the TARGET state; update() eases the rendered state to it.
    if (rotating) {
        targetYaw += deltaX * rotateSpeed;
        targetPitch += deltaY * rotateSpeed;
        targetPitch = glm::clamp(targetPitch, -89.0f, 89.0f);
    }

    if (panning) {
        // Pan axes come from the current rendered camera so panning feels
        // responsive rather than lagging behind its own smoothing.
        glm::vec3 right = glm::normalize(glm::cross(position - target, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, position - target));
        targetPanPoint -= right * deltaX * panSpeed;
        targetPanPoint += up * deltaY * panSpeed;
    }
}

void Camera::handleScrollInput(float yOffset) {
    targetDistance -= yOffset * zoomSpeed;
    targetDistance = std::max(1.0f, targetDistance);
}

void Camera::update(float deltaTime) {
    if (!smoothingEnabled) {
        distance = targetDistance;
        yaw = targetYaw;
        pitch = targetPitch;
        target = targetPanPoint;
        position = getPosition();
        return;
    }

    // Frame-rate independent exponential smoothing: feels the same at 30 and
    // 144 fps, unlike a fixed-fraction lerp.
    float t = 1.0f - std::exp(-smoothingSpeed * deltaTime);
    t = glm::clamp(t, 0.0f, 1.0f);

    distance += (targetDistance - distance) * t;
    yaw += (targetYaw - yaw) * t;
    pitch += (targetPitch - pitch) * t;
    target += (targetPanPoint - target) * t;

    position = getPosition();
}

glm::vec3 Camera::getPosition() const {
    float x = distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    float y = distance * sin(glm::radians(pitch));
    float z = distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    return glm::vec3(x, y, z) + target;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, target, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(fov, aspect, nearPlane, farPlane);
}

void Camera::setAspectRatio(float aspectRatio) {
    // A minimised window reports a 0-height framebuffer; never let that
    // produce an infinite/NaN projection matrix.
    if (aspectRatio > 0.0001f && std::isfinite(aspectRatio)) aspect = aspectRatio;
}

void Camera::focusOn(const glm::vec3& point) {
    targetPanPoint = point;
}

void Camera::reset() {
    targetDistance = 5.0f;
    targetYaw = 0.0f;
    targetPitch = 0.0f;
    targetPanPoint = glm::vec3(0.0f);
}

glm::vec2 Camera::worldToScreen(const glm::vec3& worldPos) const {
    glm::vec4 clipSpace = getProjectionMatrix() * getViewMatrix() * glm::vec4(worldPos, 1.0f);
    if (clipSpace.w == 0.0f) return glm::vec2(0.0f);

    glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w;
    return glm::vec2(ndc.x * 0.5f + 0.5f, 1.0f - (ndc.y * 0.5f + 0.5f)); // normalised [0,1], y flipped
}
