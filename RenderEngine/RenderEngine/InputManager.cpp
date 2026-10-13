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
    // On the very first sample there is no previous position; treating (0,0)
    // as "previous" produced a huge delta (camera jump) if a mouse button was
    // already held.
    if (!hasMousePos_) {
        currentMousePos = glm::vec2(xpos, ypos);
        lastMousePos = currentMousePos;
        hasMousePos_ = true;
    } else {
        lastMousePos = currentMousePos;
        currentMousePos = glm::vec2(xpos, ypos);
    }
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
    // FIX: .at() threw std::out_of_range when `button` had no previous-frame
    // entry. previousMouseButtons is empty until update() has run once, but
    // Engine::processInput() calls this BEFORE update(), so holding the left
    // button as the window first gained focus killed the app on frame 1 (the
    // exception was only logged to an invisible console). A missing entry now
    // means "was not pressed", same as isKeyJustPressed already does.
    if (!isMouseButtonPressed(button)) return false;
    auto prev = previousMouseButtons.find(button);
    const bool wasPressed = (prev != previousMouseButtons.end()) && prev->second;
    return !wasPressed;
}

glm::vec2 InputManager::getMouseDelta() const {
    return currentMousePos - lastMousePos;
}

glm::vec2 InputManager::getMousePosition() const {
    return currentMousePos;
}
