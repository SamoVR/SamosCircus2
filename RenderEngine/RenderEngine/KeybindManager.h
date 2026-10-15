#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

#include "InputManager.h"
#include "KeyCombo.h"
#include "KeyComboHash.h"

// Struct to hold a named keybind with its combo and action
struct NamedKeybind {
    std::string name;
    KeyCombo combo;
    std::function<void()> action;
};

class KeybindManager {
public:
    void update(InputManager* input);

    void registerKeybind(const std::string& name, const KeyCombo& combo, const std::function<void()>& action);
    void rebindKey(const std::string& name, const KeyCombo& newCombo);

    const std::vector<NamedKeybind>& getKeybinds() const { return namedKeybinds; }

private:
    std::vector<NamedKeybind> namedKeybinds;
};
