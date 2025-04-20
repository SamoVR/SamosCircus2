#include <functional>
#include <unordered_map>

#include "InputManager.h"
#include "KeyCombo.h"
#include "KeyComboHash.h"

class KeybindManager {
public:
    void update(InputManager* input);

    void registerKeybind(const KeyCombo& combo, const std::function<void()>& action);

private:
    std::unordered_map<KeyCombo, std::function<void()>> keybinds;
};
