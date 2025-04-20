#include "KeybindManager.h"

void KeybindManager::update(InputManager* input) {
    for (const auto& [combo, action] : keybinds) {
        if (input->isKeyJustPressed(combo.key)) {
            bool ctrl = input->isKeyPressed(GLFW_KEY_LEFT_CONTROL) || input->isKeyPressed(GLFW_KEY_RIGHT_CONTROL);
            bool shift = input->isKeyPressed(GLFW_KEY_LEFT_SHIFT) || input->isKeyPressed(GLFW_KEY_RIGHT_SHIFT);
            bool alt = input->isKeyPressed(GLFW_KEY_LEFT_ALT) || input->isKeyPressed(GLFW_KEY_RIGHT_ALT);

            if (ctrl == combo.ctrl && shift == combo.shift && alt == combo.alt) {
                action();
            }
        }
    }
}

void KeybindManager::registerKeybind(const KeyCombo& combo, const std::function<void()>& action) {
    keybinds[combo] = action;
}

