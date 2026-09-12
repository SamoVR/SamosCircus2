#include "Game.h"
#include "raylib.h"

#define _CRT_SECURE_NO_WARNINGS
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "Style.h"

Game::Game()
    : world(40, 30, 32)
{
    InitWindow(screenWidth, screenHeight, "Robot Simulation");
    SetTargetFPS(60);

    ApplyIndustrialStyle();
    world.GenerateSampleMap();
    world.SpawnRobot(RobotType::Explorer, world.GridToPixel(5, 5));
}

void Game::Run()
{
    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }
    CloseWindow();
}

void Game::Update()
{
    HandleEditorInput();

    world.SetShowPathfindingDebug(ui.ShouldShowPathDebug());

    if (!ui.IsSimulationPaused())
    {
        float deltaTime = GetFrameTime() * ui.GetSimulationSpeed();
        world.Update(deltaTime);
    }

    RobotType spawnType;
    if (ui.ShouldSpawnRobot(spawnType))
    {
        // spawn at a fixed-ish spot for now; dragging to place comes later
        world.SpawnRobot(spawnType, world.GridToPixel(3, 3));
    }
}

void Game::HandleEditorInput()
{
    Vector2 mousePos = GetMousePosition();
    if (mousePos.y <= toolbarHeight)
        return; // click is on the UI panel, not the map

    Vector2Int gridPos = world.PixelToGrid(mousePos);

    switch (ui.GetCurrentTool())
    {
    case EditTool::PlaceWall:
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            world.GetGrid().Set(gridPos.x, gridPos.y, CellType::Wall);
        break;

    case EditTool::EraseWall:
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            world.GetGrid().Set(gridPos.x, gridPos.y, CellType::Floor);
        break;

    case EditTool::PlaceGoal:
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            world.SetGoalTile(gridPos);
        break;

    case EditTool::SelectRobot:
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            world.SelectRobotAt(mousePos);
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
            world.MoveSelectedRobotTo(mousePos);
        break;

    default:
        break;
    }
}

void Game::Draw()
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    world.Draw();
    ui.Draw(world.GetSelectedRobot());

    EndDrawing();
}