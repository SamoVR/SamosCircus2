#pragma once
#include "raylib.h"
#include "Types.h"
#include <vector>
#include <deque>
#include <string>

enum class RobotType
{
    Explorer,
    Carrier,
    Patrol
};

class Robot
{
public:
    Robot(Vector2 startPixelPos);
    virtual ~Robot() = default;

    virtual void Update(float deltaTime);
    virtual void Draw(bool isSelected) const;

    virtual RobotType GetType() const = 0;      // pure virtual - forces subclasses to define this
    virtual const char* GetTypeName() const = 0;
    virtual Color GetColor() const = 0;

    void SetPath(const std::vector<Vector2>& pixelWaypoints);
    bool IsMoving() const;

    Vector2 GetPosition() const { return position; }

protected:
    Vector2 position;
    std::deque<Vector2> path;
    float speed = 120.0f;
};