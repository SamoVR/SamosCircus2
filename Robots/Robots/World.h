#pragma once
#include "Grid.h"
#include "Types.h"
#include "Robot.h"
#include "Pathfinder.h"
#include "raylib.h"
#include <vector>
#include <memory>

class World
{
public:
    World(int gridWidth, int gridHeight, int cellSize);

    void GenerateSampleMap();
    void Update(float deltaTime);
    void Draw() const;

    Grid& GetGrid() { return grid; }
    int GetCellSize() const { return cellSize; }

    Vector2 GridToPixel(int gx, int gy) const;
    Vector2Int PixelToGrid(Vector2 pixelPos) const;

    // robot management
    void SpawnRobot(RobotType type, Vector2 pixelPos);
    void SelectRobotAt(Vector2 pixelPos); // click-to-select, nullptr-equivalent if none hit
    void MoveSelectedRobotTo(Vector2 clickedPixelPos);
    Robot* GetSelectedRobot() const { return selectedRobot; }

    // goal tile
    void SetGoalTile(Vector2Int gridPos);
    bool HasGoal() const { return hasGoal; }
    Vector2Int GetGoalTile() const { return goalTile; }

    // visualization toggle
    void SetShowPathfindingDebug(bool show) { showPathfindingDebug = show; }

private:
    Grid grid;
    int cellSize;

    std::vector<std::unique_ptr<Robot>> robots;
    Robot* selectedRobot = nullptr;

    bool hasGoal = false;
    Vector2Int goalTile{ 0, 0 };

    std::vector<Vector2Int> lastVisitedNodes; // for A* debug overlay
    bool showPathfindingDebug = false;

    void DrawMap() const;
    void DrawRobots() const;
    void DrawGoalTile() const;
    void DrawPathfindingDebug() const;
};