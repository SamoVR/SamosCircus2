#include "InputManager.h"

void InputManager::update(GLFWwindow* window) {

    // Store previous input state
    previousKeys = currentKeys;
    previousMouseButtons = currentMouseButtons;

    // Update key states
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
        currentKeys[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }

    // Update mouse button states
    for (int button = GLFW_MOUSE_BUTTON_1; button <= GLFW_MOUSE_BUTTON_8; ++button) {
        currentMouseButtons[button] = glfwGetMouseButton(window, button) == GLFW_PRESS;
    }

    // Update mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    lastMousePos = currentMousePos;
    currentMousePos = glm::vec2(xpos, ypos);
}

bool InputManager::isKeyPressed(int key) const {
    auto it = currentKeys.find(key);
    return it != currentKeys.end() && it->second;
}

bool InputManager::isKeyJustPressed(int key) const {
    auto curr = currentKeys.find(key);
    auto prev = previousKeys.find(key);

    return curr != currentKeys.end() && curr->second &&
        (prev == previousKeys.end() || !prev->second);
}

bool InputManager::isMouseButtonPressed(int button) const {
    auto it = currentMouseButtons.find(button);
    return it != currentMouseButtons.end() && it->second;
}

bool InputManager::isMouseButtonJustPressed(int button) const {
    return isMouseButtonPressed(button) && !previousMouseButtons.at(button);
}

glm::vec2 InputManager::getMouseDelta() const {
    return currentMousePos - lastMousePos;
}

glm::vec2 InputManager::getMousePosition() const {
    return currentMousePos;
}
