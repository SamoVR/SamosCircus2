#pragma once

struct Vector2Int
{
    int x, y;

    bool operator==(const Vector2Int& other) const
    {
        return x == other.x && y == other.y;
    }
};