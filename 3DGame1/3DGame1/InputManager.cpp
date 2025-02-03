#include "InputManager.h"
#include "Camera.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>  
#include <imgui.h>

#include <iostream>

void InputManager::handleKeyboard(GLFWwindow* window, Camera& camera, float deltaTime)
{
    //// MOVEMENT ////

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::SPACE,deltaTime);

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::SHIFT, deltaTime); //might be a bad way to have it done like this but wtv fuck optimization
    else
        camera.processKeyboard(CameraMovement::SHIFT_RELEASED, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.processKeyboard(CameraMovement::CTRL, deltaTime);

    //////////////////

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && cursorLocked) {
        cursorLocked = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // Unlock cursor
        std::cout << "Cursor unlocked." << std::endl;
    }
}

void InputManager::handleMouse(GLFWwindow* window)
{
    ImGuiIO& io = ImGui::GetIO();

    if (!cursorLocked && !io.WantCaptureMouse && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        cursorLocked = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Lock cursor
        std::cout << "Cursor locked." << std::endl;
    }
}