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
    RIGHT,

    SPACE,
    SHIFT,
    SHIFT_RELEASED,
    CTRL
};

class Camera {
public:

    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 characterFront;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Constructor
    Camera(glm::vec3 position, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
        float yaw = -90.0f, float pitch = 0.0f);

    // Getters
    glm::mat4 getProjection(float width,float height) const;
    glm::mat4 getViewMatrix() const;
    float getFOV() const;
    float getMovementSpeed() const;

    // Setters
    void setMovementSpeed(float speed);

    // Process inputs
    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // Static mouse callback for GLFW
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos);

    // Set initial mouse position
    void setInitialMousePosition(float x, float y);

    bool isCrouching = false;
    float standHeight = 2.0f;
    float crouchHeight = 1.25f;

private:
    // Camera attributes
    //moved to public

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
