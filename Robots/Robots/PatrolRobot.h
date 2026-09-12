#pragma once
#include "Robot.h"

class PatrolRobot : public Robot
{
public:
    PatrolRobot(Vector2 startPixelPos) : Robot(startPixelPos) {}

    RobotType GetType() const override { return RobotType::Patrol; }
    const char* GetTypeName() const override { return "Patrol"; }
    Color GetColor() const override { return Color { 140, 80, 200, 255 }; } // purple
};