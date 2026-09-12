#include "Pathfinder.h"
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

struct Vector2IntHash
{
    size_t operator()(const Vector2Int& v) const
    {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};

static float Heuristic(Vector2Int a, Vector2Int b)
{
    return (float)(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

struct Node
{
    Vector2Int pos;
    float fCost;

    bool operator>(const Node& other) const
    {
        return fCost > other.fCost;
    }
};

PathResult Pathfinder::FindPath(const Grid& grid, Vector2Int start, Vector2Int goal)
{
    PathResult result;

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
    std::unordered_map<Vector2Int, Vector2Int, Vector2IntHash> cameFrom;
    std::unordered_map<Vector2Int, float, Vector2IntHash> gCost;
    std::unordered_map<Vector2Int, bool, Vector2IntHash> visitedSet;

    openSet.push({ start, Heuristic(start, goal) });
    gCost[start] = 0.0f;

    const Vector2Int directions[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

    while (!openSet.empty())
    {
        Vector2Int current = openSet.top().pos;
        openSet.pop();

        if (visitedSet.find(current) != visitedSet.end())
            continue; // already processed this one via a cheaper route
        visitedSet[current] = true;
        result.visited.push_back(current);

        if (current == goal)
        {
            Vector2Int step = current;
            while (!(step == start))
            {
                result.path.push_back(step);
                step = cameFrom[step];
            }
            std::reverse(result.path.begin(), result.path.end());
            return result;
        }

        for (const auto& dir : directions)
        {
            Vector2Int neighbor = { current.x + dir.x, current.y + dir.y };

            if (!grid.IsWalkable(neighbor.x, neighbor.y))
                continue;

            float tentativeG = gCost[current] + 1.0f;

            if (gCost.find(neighbor) == gCost.end() || tentativeG < gCost[neighbor])
            {
                gCost[neighbor] = tentativeG;
                float fCost = tentativeG + Heuristic(neighbor, goal);
                openSet.push({ neighbor, fCost });
                cameFrom[neighbor] = current;
            }
        }
    }

    return result; // path stays empty - no path found, but visited is still filled
}