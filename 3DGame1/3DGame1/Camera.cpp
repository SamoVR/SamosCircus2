#include "Camera.h"
#include "Window.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(front);

    this->front = front;

    glm::vec3 front2;
    front2.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front2.y = 0.0f;
    front2.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    characterFront = glm::normalize(front2);

    this->characterFront = characterFront;

    // Calculate the right and up vectors
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

// Constructor with initial values
Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : position(position), worldUp(up), yaw(yaw), pitch(pitch),
    movementSpeed(normalMovementSpeed), mouseSensitivity(0.1f), fov(60.0f),
    lastX(400), lastY(300), firstMouse(true) {
    updateCameraVectors();
}

// Processes input received from keyboard
void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = movementSpeed * deltaTime;

    if (direction == CameraMovement::FORWARD)
        position += characterFront * velocity;
    if (direction == CameraMovement::BACKWARD)
        position -= characterFront * velocity;
    if (direction == CameraMovement::LEFT)
        position -= right * velocity;
    if (direction == CameraMovement::RIGHT)
        position += right * velocity;

    if (direction == CameraMovement::SHIFT) //crouching
    {
        movementSpeed = crouchMovementSpeed;
        position.y = crouchHeight;
    }
    if (direction == CameraMovement::SHIFT_RELEASED) //standing
    {
        movementSpeed = normalMovementSpeed;
        position.y = normalHeight;
    }

    if (direction == CameraMovement::CTRL) //run
    {
        movementSpeed = runMovementSpeed;
    }
        

}

// Processes input received from mouse movement
void Camera::processMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch = glm::clamp(pitch + yoffset, -89.0f, 89.0f); //limit is 89

    /*if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }*/



    updateCameraVectors();
}

// Static GLFW mouse callback
void Camera::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera->firstMouse) {
        camera->lastX = xpos;
        camera->lastY = ypos;
        camera->firstMouse = false;
    }

    float xoffset = xpos - camera->lastX;
    float yoffset = camera->lastY - ypos; // Reversed since y-coordinates go from bottom to top
    camera->lastX = xpos;
    camera->lastY = ypos;

    camera->processMouseMovement(xoffset, yoffset);
}

// Set initial mouse position (used for resetting)
void Camera::setInitialMousePosition(float x, float y) {
    lastX = x;
    lastY = y;
}

/* Getters */

// Returns the view matrix using LookAt matrix
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjection(float width, float height) const {
    return glm::perspective(glm::radians(fov), (float)width / height, 0.1f, 100.0f);
}

float Camera::getFOV() const {
    return fov;
}

float Camera::getMovementSpeed() const {
    return movementSpeed;
}

/* Setters */

void Camera::setMovementSpeed(float newSpeed) {
    movementSpeed = newSpeed;
}
