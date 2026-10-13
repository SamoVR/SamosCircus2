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

std::string KeybindManager::findConflict(const KeyCombo& combo, const std::string& excludingName) const {
    for (const auto& keybind : namedKeybinds) {
        if (keybind.name == excludingName) continue;
        if (keybind.combo == combo) return keybind.name;
    }
    return "";
}

std::string KeybindManager::rebindKey(const std::string& name, const KeyCombo& newCombo) {
    const std::string conflict = findConflict(newCombo, name);
    for (auto& keybind : namedKeybinds) {
        if (keybind.name == name) {
            keybind.combo = newCombo;
            break;
        }
    }
    return conflict;
}
