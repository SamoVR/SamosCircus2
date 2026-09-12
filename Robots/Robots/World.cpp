#include "World.h"
#include "ExplorerRobot.h"
#include "CarrierRobot.h"
#include "PatrolRobot.h"
#include "Theme.h"
#include <raymath.h>

World::World(int gridWidth, int gridHeight, int cellSize)
    : grid(gridWidth, gridHeight), cellSize(cellSize)
{
}

void World::GenerateSampleMap()
{
    int w = grid.GetWidth();
    int h = grid.GetHeight();

    for (int x = 0; x < w; x++)
    {
        grid.Set(x, 0, CellType::Wall);
        grid.Set(x, h - 1, CellType::Wall);
    }
    for (int y = 0; y < h; y++)
    {
        grid.Set(0, y, CellType::Wall);
        grid.Set(w - 1, y, CellType::Wall);
    }

    for (int x = 10; x < 20; x++)
    {
        grid.Set(x, 8, CellType::Wall);
        grid.Set(x, 15, CellType::Wall);
    }
    for (int y = 8; y < 15; y++)
    {
        grid.Set(10, y, CellType::Wall);
        grid.Set(19, y, CellType::Wall);
    }
    grid.Set(19, 11, CellType::Floor);
    grid.Set(19, 12, CellType::Floor);
}

void World::Update(float deltaTime)
{
    for (auto& robot : robots)
        robot->Update(deltaTime);
}

void World::SpawnRobot(RobotType type, Vector2 pixelPos)
{
    switch (type)
    {
    case RobotType::Explorer:
        robots.push_back(std::make_unique<ExplorerRobot>(pixelPos));
        break;
    case RobotType::Carrier:
        robots.push_back(std::make_unique<CarrierRobot>(pixelPos));
        break;
    case RobotType::Patrol:
        robots.push_back(std::make_unique<PatrolRobot>(pixelPos));
        break;
    }
}

void World::SelectRobotAt(Vector2 pixelPos)
{
    selectedRobot = nullptr;
    const float clickRadius = 16.0f;

    for (auto& robot : robots)
    {
        if (Vector2Distance(robot->GetPosition(), pixelPos) <= clickRadius)
        {
            selectedRobot = robot.get();
            return;
        }
    }
}

void World::MoveSelectedRobotTo(Vector2 clickedPixelPos)
{
    if (!selectedRobot)
        return;

    Vector2Int startGrid = PixelToGrid(selectedRobot->GetPosition());
    Vector2Int goalGrid = PixelToGrid(clickedPixelPos);

    PathResult result = Pathfinder::FindPath(grid, startGrid, goalGrid);
    lastVisitedNodes = result.visited;

    if (result.path.empty())
        return;

    std::vector<Vector2> pixelPath;
    for (const auto& gp : result.path)
        pixelPath.push_back(GridToPixel(gp.x, gp.y));

    selectedRobot->SetPath(pixelPath);
}

void World::SetGoalTile(Vector2Int gridPos)
{
    hasGoal = true;
    goalTile = gridPos;

    if (selectedRobot)
        MoveSelectedRobotTo(GridToPixel(gridPos.x, gridPos.y));
}

Vector2 World::GridToPixel(int gx, int gy) const
{
    return {
        gx * (float)cellSize + cellSize / 2.0f,
        gy * (float)cellSize + cellSize / 2.0f
    };
}

Vector2Int World::PixelToGrid(Vector2 pixelPos) const
{
    return {
        (int)(pixelPos.x / cellSize),
        (int)(pixelPos.y / cellSize)
    };
}

void World::Draw() const
{
    DrawMap();

    if (showPathfindingDebug)
        DrawPathfindingDebug();

    if (hasGoal)
        DrawGoalTile();

    DrawRobots();
}

void World::DrawMap() const
{
    for (int y = 0; y < grid.GetHeight(); y++)
    {
        for (int x = 0; x < grid.GetWidth(); x++)
        {
            Color color = (grid.Get(x, y) == CellType::Wall)
                ? Theme::WallTile
                : Theme::FloorTile;

            Rectangle rect = {
                (float)(x * cellSize), (float)(y * cellSize),
                (float)cellSize, (float)cellSize
            };
            DrawRectangleRec(rect, color);
        }
    }
}

void World::DrawRobots() const
{
    for (auto& robot : robots)
        robot->Draw(robot.get() == selectedRobot);
}

void World::DrawGoalTile() const
{
    Rectangle rect = {
        (float)(goalTile.x * cellSize), (float)(goalTile.y * cellSize),
        (float)cellSize, (float)cellSize
    };
    DrawRectangleRec(rect, Color { 80, 200, 100, 180 });
}

void World::DrawPathfindingDebug() const
{
    for (const auto& node : lastVisitedNodes)
    {
        Rectangle rect = {
            (float)(node.x * cellSize), (float)(node.y * cellSize),
            (float)cellSize, (float)cellSize
        };
        DrawRectangleRec(rect, Color { 255, 100, 100, 60 }); // translucent red
    }
}