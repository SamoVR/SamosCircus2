#pragma once
#include "Grid.h"
#include "Types.h"
#include <vector>

struct PathResult
{
    std::vector<Vector2Int> path;      // empty if no path found
    std::vector<Vector2Int> visited;   // every node explored, in order - for visualization
};

class Pathfinder
{
public:
    static PathResult FindPath(const Grid& grid, Vector2Int start, Vector2Int goal);
};