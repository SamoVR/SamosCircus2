#include "Camera.h"

Camera::Camera(float aspectRatio)
    : position(0.0f, 0.0f, 5.0f),  // Set initial position (can be adjusted later)
    target(0.0f), distance(5.0f), yaw(0.0f), pitch(0.0f),
    zoomSpeed(1.0f), rotateSpeed(0.3f), panSpeed(0.005f),
    fov(glm::radians(45.0f)), aspect(aspectRatio), nearPlane(0.1f), farPlane(100.0f) {

}

void Camera::handleMouseInput(float deltaX, float deltaY, bool rotating, bool panning) {
    if (rotating) {
        yaw += deltaX * rotateSpeed;
        pitch += deltaY * rotateSpeed;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
    }

    if (panning) {
        glm::vec3 right = glm::normalize(glm::cross(getPosition() - target, glm::vec3(0.0f, 1.0f, 0.0f)));
        glm::vec3 up = glm::normalize(glm::cross(right, getPosition() - target));
        target -= right * deltaX * panSpeed;
        target += up * deltaY * panSpeed;
    }

    position = getPosition();
}

void Camera::handleScrollInput(float yOffset) {
    distance -= yOffset * zoomSpeed;
    distance = std::max(1.0f, distance);

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
    aspect = aspectRatio;  // Update the aspect ratio
}

void Camera::update(float deltaTime) {
    // No time-based motion for now
}

glm::vec2 Camera::worldToScreen(const glm::vec3& worldPos) const {
    glm::mat4 view = getViewMatrix();
    glm::mat4 projection = getProjectionMatrix();
    glm::vec4 clipSpace = projection * view * glm::vec4(worldPos, 1.0f);

    if (clipSpace.w == 0.0f) return glm::vec2(0.0f); // avoid divide-by-zero

    glm::vec3 ndc = glm::vec3(clipSpace) / clipSpace.w; // Normalize

    // Convert NDC [-1,1] to screen space [0, window]
    float screenX = (ndc.x * 0.5f + 0.5f); // multiply by width later
    float screenY = (1.0f - (ndc.y * 0.5f + 0.5f)); // flip y

    return glm::vec2(screenX, screenY); // still normalized [0,1]
}

