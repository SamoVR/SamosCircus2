// InputManager.h
#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>
#include <glm/glm.hpp>

class InputManager {
public:
    void update(GLFWwindow* window);

    bool isKeyPressed(int key) const;
    bool isKeyJustPressed(int key) const;

    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonJustPressed(int button) const;

    glm::vec2 getMouseDelta() const;
    glm::vec2 getMousePosition() const;

    float getScrollOffset() const;
    void setScrollCallback(GLFWwindow* window);

private:
    std::unordered_map<int, bool> currentKeys;
    std::unordered_map<int, bool> previousKeys;

    std::unordered_map<int, bool> currentMouseButtons;
    std::unordered_map<int, bool> previousMouseButtons;

    glm::vec2 lastMousePos = glm::vec2(0.0f);
    glm::vec2 currentMousePos = glm::vec2(0.0f);

    float scrollOffset = 0.0f;

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};
