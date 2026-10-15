#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

#include "InputManager.h"
#include "KeyCombo.h"
#include "KeyComboHash.h"

struct NamedKeybind {
    std::string name;
    KeyCombo combo;
    std::function<void()> action;
};

class KeybindManager {
public:
    void update(InputManager* input);

    void registerKeybind(const std::string& name, const KeyCombo& combo, const std::function<void()>& action);

    // Rebinds `name`. Returns the name of ANOTHER keybind already using that
    // exact combo ("" if none). The rebind is still applied -- what to do
    // about a conflict (warn, block, undo) is the UI layer's decision. The
    // original let two actions silently share one combo so both fired.
    std::string rebindKey(const std::string& name, const KeyCombo& newCombo);

    // Name of a keybind (other than `excludingName`) already using `combo`.
    std::string findConflict(const KeyCombo& combo, const std::string& excludingName = "") const;

    const std::vector<NamedKeybind>& getKeybinds() const { return namedKeybinds; }

private:
    std::vector<NamedKeybind> namedKeybinds;
};
