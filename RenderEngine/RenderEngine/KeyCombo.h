#pragma once

#include <GLFW/glfw3.h>

struct KeyCombo {
    int key;
    bool ctrl;
    bool shift;
    bool alt;

    bool operator==(const KeyCombo& other) const {
        return key == other.key &&
            ctrl == other.ctrl &&
            shift == other.shift &&
            alt == other.alt;
    }
};
