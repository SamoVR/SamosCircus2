#include "Grid.h"

Grid::Grid(int width, int height)
    : width(width), height(height), cells(width* height, CellType::Floor)
{
}

CellType Grid::Get(int x, int y) const
{
    if (!InBounds(x, y))
        return CellType::Wall; // mimo mapy = považuj za stenu (bezpečná default hodnota)

    return cells[y * width + x];
}

void Grid::Set(int x, int y, CellType type)
{
    if (InBounds(x, y))
        cells[y * width + x] = type;
}

bool Grid::IsWalkable(int x, int y) const
{
    return InBounds(x, y) && Get(x, y) != CellType::Wall;
}

bool Grid::InBounds(int x, int y) const
{
    return x >= 0 && y >= 0 && x < width && y < height;
}