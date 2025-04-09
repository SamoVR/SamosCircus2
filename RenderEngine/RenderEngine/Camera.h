#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Camera {
public:
    Camera(float aspectRatio);

    void update(float deltaTime);
    void handleMouseInput(float deltaX, float deltaY, bool rotating, bool panning);
    void handleScrollInput(float yOffset);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::vec3 target;

    void setAspectRatio(float aspectRatio);  // New method to set aspect ratio

private:
    float distance;
    float yaw;
    float pitch;
    float zoomSpeed;
    float rotateSpeed;
    float panSpeed;

    float fov;
    float aspect;
    float nearPlane;
    float farPlane;

    glm::vec3 getPosition() const;
};
