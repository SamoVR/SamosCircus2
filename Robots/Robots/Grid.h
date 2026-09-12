#pragma once
#include <vector>

enum class CellType
{
    Floor,
    Wall
};

class Grid
{
public:
    Grid(int width, int height);

    CellType Get(int x, int y) const;
    void Set(int x, int y, CellType type);
    bool IsWalkable(int x, int y) const;
    bool InBounds(int x, int y) const;

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

private:
    int width;
    int height;
    std::vector<CellType> cells;
};