// Theme.h
#pragma once
#include "raylib.h"

namespace Theme
{
    inline const Color Background = { 26, 29, 31, 255 };
    inline const Color Panel = { 37, 42, 45, 255 };
    inline const Color Border = { 58, 65, 69, 255 };
    inline const Color TextPrimary = { 232, 230, 224, 255 };
    inline const Color TextMuted = { 150, 150, 145, 255 };
    inline const Color AccentAmber = { 212, 162, 76, 255 };
    inline const Color AccentBlue = { 91, 155, 213, 255 };

    inline const Color FloorTile = { 60, 64, 62, 255 };   // dark industrial floor
    inline const Color WallTile = { 20, 22, 23, 255 };   // near-black walls
    inline const Color GoalTile = { 212, 162, 76, 180 }; // amber, translucent
    inline const Color PathDebug = { 212, 162, 76, 60 };  // amber, more translucent
}