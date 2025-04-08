#include "Camera.h"

Camera::Camera(float aspectRatio)
    : target(0.0f), distance(5.0f), yaw(0.0f), pitch(0.0f),
    zoomSpeed(1.0f), rotateSpeed(0.3f),
    fov(glm::radians(45.0f)), aspect(aspectRatio), nearPlane(0.1f), farPlane(100.0f) {
}

void Camera::handleMouseInput(float deltaX, float deltaY, bool rotating) {
    if (rotating) {
        yaw += deltaX * rotateSpeed;
        pitch += deltaY * rotateSpeed;
        pitch = glm::clamp(pitch, -89.0f, 89.0f); // prevent flip
    }
}

void Camera::handleScrollInput(float yOffset) {
    distance -= yOffset * zoomSpeed;
    distance = std::max(1.0f, distance);
}

glm::vec3 Camera::getPosition() const {
    float x = distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    float y = distance * sin(glm::radians(pitch));
    float z = distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    return glm::vec3(x, y, z) + target;
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(getPosition(), target, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(fov, aspect, nearPlane, farPlane);
}

void Camera::update(float deltaTime) {
    // No time-based motion for now
}
