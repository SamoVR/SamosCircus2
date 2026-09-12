#pragma once
#include "Robot.h"

class ExplorerRobot : public Robot
{
public:
    ExplorerRobot(Vector2 startPixelPos) : Robot(startPixelPos) {}

    RobotType GetType() const override { return RobotType::Explorer; }
    const char* GetTypeName() const override { return "Explorer"; }
    Color GetColor() const override { return Color { 60, 140, 220, 255 }; } // blue
};