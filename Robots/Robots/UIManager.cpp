#include "UIManager.h"
#include "raylib.h"
#include "raygui.h"
#include "Theme.h"

void UIManager::Draw(Robot* selectedRobot)
{
    const int panelHeight = 90;
    DrawRectangle(0, 0, GetScreenWidth(), panelHeight, Theme::Panel);

    // --- row 1: editing tools ---
    if (GuiButton(Rectangle { 10, 10, 80, 28 }, "Wall"))
        currentTool = EditTool::PlaceWall;
    if (GuiButton(Rectangle { 95, 10, 80, 28 }, "Erase"))
        currentTool = EditTool::EraseWall;
    if (GuiButton(Rectangle { 180, 10, 80, 28 }, "Goal"))
        currentTool = EditTool::PlaceGoal;
    if (GuiButton(Rectangle { 265, 10, 80, 28 }, "Select"))
        currentTool = EditTool::SelectRobot;

    // --- row 1 continued: robot spawning ---
    if (GuiButton(Rectangle { 370, 10, 90, 28 }, "+ Explorer"))
    {
        spawnRequested = true;
        requestedType = RobotType::Explorer;
    }
    if (GuiButton(Rectangle { 465, 10, 90, 28 }, "+ Carrier"))
    {
        spawnRequested = true;
        requestedType = RobotType::Carrier;
    }
    if (GuiButton(Rectangle { 560, 10, 90, 28 }, "+ Patrol"))
    {
        spawnRequested = true;
        requestedType = RobotType::Patrol;
    }

    // --- row 2: simulation controls ---
    bool pausedState = isPaused;
    GuiCheckBox(Rectangle { 10, 50, 20, 20 }, "Paused", & pausedState);
    isPaused = pausedState;

    GuiSlider(Rectangle { 150, 50, 150, 20 }, "Speed", TextFormat("%.1fx", speed), & speed, 0.1f, 3.0f);

    bool debugState = showPathDebug;
    GuiCheckBox(Rectangle { 320, 50, 20, 20 }, "Show A* Debug", & debugState);
    showPathDebug = debugState;

    // --- sidebar: selected robot info ---
    int sw = GetScreenWidth();
    DrawRectangle(sw - 220, panelHeight, 220, 150, Color { 40, 40, 45, 200 });

    if (selectedRobot)
    {
        DrawText("Selected Robot", sw - 210, panelHeight + 10, 16, Theme::TextPrimary);
        DrawText(TextFormat("Type: %s", selectedRobot->GetTypeName()), sw - 210, panelHeight + 35, 14, LIGHTGRAY);
        DrawText(TextFormat("Moving: %s", selectedRobot->IsMoving() ? "Yes" : "No"), sw - 210, panelHeight + 55, 14, LIGHTGRAY);
    }
    else
    {
        DrawText("No robot selected", sw - 210, panelHeight + 10, 16, GRAY);
    }
}

bool UIManager::ShouldSpawnRobot(RobotType& outType)
{
    if (spawnRequested)
    {
        outType = requestedType;
        spawnRequested = false; // consume the request
        return true;
    }
    return false;
}