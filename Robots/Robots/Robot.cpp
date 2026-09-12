#include "Robot.h"
#include "raymath.h"

Robot::Robot(Vector2 startPixelPos)
    : position(startPixelPos)
{
}

void Robot::SetPath(const std::vector<Vector2>& pixelWaypoints)
{
    path.assign(pixelWaypoints.begin(), pixelWaypoints.end());
}

bool Robot::IsMoving() const
{
    return !path.empty();
}

void Robot::Update(float deltaTime)
{
    if (path.empty())
        return;

    Vector2 currentTarget = path.front();
    float distance = Vector2Distance(position, currentTarget);

    if (distance < 2.0f)
    {
        position = currentTarget;
        path.pop_front();
        return;
    }

    Vector2 direction = Vector2Normalize(Vector2Subtract(currentTarget, position));
    position = Vector2Add(position, Vector2Scale(direction, speed * deltaTime));
}

void Robot::Draw(bool isSelected) const
{
    Color color = GetColor();
    DrawCircleV(position, 12.0f, color);
    DrawCircleLines((int)position.x, (int)position.y, 12.0f, BLACK);

    if (isSelected)
        DrawCircleLines((int)position.x, (int)position.y, 16.0f, YELLOW);
}