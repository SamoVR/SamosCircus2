#ifndef CAMERA_H
#define CAMERA_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Enum for camera movement
enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

class Camera {
public:
    // Constructor
    Camera(glm::vec3 position, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = 0.0f);

    // Getters
    glm::mat4 getProjection(float width,float height) const;
    glm::mat4 getViewMatrix() const;
    float getFOV() const;
    float getMovementSpeed() const;

    // Process inputs
    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // Static mouse callback for GLFW
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

    // Set initial mouse position
    void setInitialMousePosition(float x, float y);

private:
    // Camera attributes
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 characterFront;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler angles
    float yaw;
    float pitch;

    // Camera options
    float movementSpeed;
    float mouseSensitivity;
    float fov;

    // Last mouse position
    float lastX, lastY;
    bool firstMouse;

    // Recalculate the camera vectors
    void updateCameraVectors();
};

#endif // CAMERA_H
