#include "KeybindManager.h"

void KeybindManager::update(InputManager* input) {
    for (const auto& keybind : namedKeybinds) {
        if (input->isKeyJustPressed(keybind.combo.key)) {
            bool ctrl = input->isKeyPressed(GLFW_KEY_LEFT_CONTROL) || input->isKeyPressed(GLFW_KEY_RIGHT_CONTROL);
            bool shift = input->isKeyPressed(GLFW_KEY_LEFT_SHIFT) || input->isKeyPressed(GLFW_KEY_RIGHT_SHIFT);
            bool alt = input->isKeyPressed(GLFW_KEY_LEFT_ALT) || input->isKeyPressed(GLFW_KEY_RIGHT_ALT);

            if (ctrl == keybind.combo.ctrl && shift == keybind.combo.shift && alt == keybind.combo.alt) {
                keybind.action();
            }
        }
    }
}

void KeybindManager::registerKeybind(const std::string& name, const KeyCombo& combo, const std::function<void()>& action) {
    for (auto& keybind : namedKeybinds) {
        if (keybind.name == name) {
            keybind.combo = combo;
            keybind.action = action;
            return;
        }
    }
    namedKeybinds.push_back({ name, combo, action });
}


void KeybindManager::rebindKey(const std::string& name, const KeyCombo& newCombo) {
    for (auto& keybind : namedKeybinds) {
        if (keybind.name == name) {
            keybind.combo = newCombo;
            //std::cout << "Rebound " << name << " to key " << newCombo.key << std::endl;
            break;
        }
    }
}
