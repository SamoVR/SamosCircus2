#pragma once

#include "KeyCombo.h"
#include <functional>

namespace std {
    template<>
    struct hash<KeyCombo> {
        std::size_t operator()(const KeyCombo& k) const {
            size_t h1 = hash<int>()(k.key);
            size_t h2 = hash<bool>()(k.ctrl);
            size_t h3 = hash<bool>()(k.shift);
            size_t h4 = hash<bool>()(k.alt);
            return ((h1 ^ (h2 << 1)) >> 1) ^ (h3 << 1) ^ (h4 << 2);
        }
    };
}
