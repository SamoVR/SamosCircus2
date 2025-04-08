#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Camera {
public:
    Camera(float aspectRatio);

    void update(float deltaTime);
    void handleMouseInput(float deltaX, float deltaY, bool rotating);
    void handleScrollInput(float yOffset);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

private:
    glm::vec3 target;
    float distance;
    float yaw;
    float pitch;
    float zoomSpeed;
    float rotateSpeed;

    float fov;
    float aspect;
    float nearPlane;
    float farPlane;

    glm::vec3 getPosition() const;
};
