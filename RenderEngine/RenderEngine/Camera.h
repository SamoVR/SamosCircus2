#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

class Camera {
public:
    Camera(float aspectRatio);

    // Eases the rendered camera toward the input-driven target state. Must be
    // called once per frame with the real delta time (Engine's main loop).
    // It used to be an empty stub that nothing called.
    void update(float deltaTime);

    void handleMouseInput(float deltaX, float deltaY, bool rotating, bool panning);
    void handleScrollInput(float yOffset);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    glm::vec3 target;
    glm::vec3 position;

    void setAspectRatio(float aspectRatio);

    // Moves the orbit pivot to `point` (eased when smoothing is on).
    void focusOn(const glm::vec3& point);
    // Back to the startup view.
    void reset();
    glm::vec2 worldToScreen(const glm::vec3& worldPos) const;

    // Smooth (eased) orbit/pan/zoom vs. instant response. Toggle in the UI.
    bool smoothingEnabled = true;
    // Higher = snappier. 12 reaches ~90% of the way in about 0.2 s.
    float smoothingSpeed = 12.0f;

private:
    // Rendered state.
    float distance;
    float yaw;
    float pitch;

    // Input-driven target state that update() eases toward.
    float targetDistance;
    float targetYaw;
    float targetPitch;
    glm::vec3 targetPanPoint;

    float zoomSpeed;
    float rotateSpeed;
    float panSpeed;

    float fov;
    float aspect;
    float nearPlane;
    float farPlane;

    glm::vec3 getPosition() const;
};
