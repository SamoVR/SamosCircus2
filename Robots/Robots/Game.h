#pragma once
#include "World.h"
#include "UIManager.h"

class Game
{
public:
    Game();
    void Run();

private:
    void Update();
    void Draw();
    void HandleEditorInput();

    const int screenWidth = 1280;
    const int screenHeight = 800;
    const int toolbarHeight = 90;

    World world;
    UIManager ui;
};