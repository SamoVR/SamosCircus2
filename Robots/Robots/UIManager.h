#pragma once
#include "Robot.h"

enum class EditTool
{
    None,
    PlaceWall,
    EraseWall,
    PlaceGoal,
    SelectRobot
};

class UIManager
{
public:
    void Draw(Robot* selectedRobot);

    EditTool GetCurrentTool() const { return currentTool; }
    bool IsSimulationPaused() const { return isPaused; }
    float GetSimulationSpeed() const { return speed; }
    bool ShouldSpawnRobot(RobotType& outType); // returns true once, then resets - "consumed" pattern
    bool ShouldShowPathDebug() const { return showPathDebug; }

private:
    EditTool currentTool = EditTool::PlaceWall;
    bool isPaused = false;
    float speed = 1.0f;
    bool showPathDebug = true;

    bool spawnRequested = false;
    RobotType requestedType = RobotType::Explorer;
};