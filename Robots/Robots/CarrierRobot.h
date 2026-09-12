#pragma once
#include "Robot.h"

class CarrierRobot : public Robot
{
public:
    CarrierRobot(Vector2 startPixelPos) : Robot(startPixelPos) {}

    RobotType GetType() const override { return RobotType::Carrier; }
    const char* GetTypeName() const override { return "Carrier"; }
    Color GetColor() const override { return Color { 220, 160, 40, 255 }; } // orange
};