#include "Game.h"
#include <iostream>
#include <string>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

Game::Game()
{
	InitWindow(screenWidth, screenHeight, "Pog");
	SetTargetFPS(60);
}

void Game::Run()
{
	while (!windowClose)
	{
		if (IsKeyPressed(KEY_ESCAPE) || WindowShouldClose()) windowClose = true;

		Update();
		Draw();
	}
	CloseWindow();
}

void Game::Update()
{
    // Left wall
    if (IsKeyDown(KEY_W) && leftWallY > 0 && !gameOver)
        leftWallY -= 5;

    if (IsKeyDown(KEY_S) && leftWallY < screenHeight - wallHeight && !gameOver)
        leftWallY += 5;

    // Right wall
    if (IsKeyDown(KEY_UP) && rightWallY > 0 && !gameOver)
        rightWallY -= 5;

    if (IsKeyDown(KEY_DOWN) && rightWallY < screenHeight - wallHeight && !gameOver)
        rightWallY += 5;

    // Move ball
    ballPosition.x += ballVelocity.x;
    ballPosition.y += ballVelocity.y;

    // Top / bottom collision
    if (ballPosition.y - ballSize <= 0)
    {
        ballPosition.y = ballSize;
        ballVelocity.y = fabs(ballVelocity.y);
    }
    else if (ballPosition.y + ballSize >= screenHeight)
    {
        ballPosition.y = screenHeight - ballSize;
        ballVelocity.y = -fabs(ballVelocity.y);
    }

    // wall rectangles
    Rectangle leftWall{
        100.0f,
        leftWallY,
        25.0f,
        (float)wallHeight
    };

    Rectangle rightWall{
        900.0f,
        rightWallY,
        25.0f,
        (float)wallHeight
    };

    // Ball collision rectangle
    Rectangle ballRect{
        ballPosition.x - ballSize,
        ballPosition.y - ballSize,
        ballSize * 2.0f,
        ballSize * 2.0f
    };

    // Left wall collision
    if (CheckCollisionRecs(ballRect, leftWall) && ballVelocity.x < 0)
    {
        ballPosition.x = leftWall.x + leftWall.width + ballSize;
        ballVelocity.x *= -1.1f;
        bumpCount++;
    }

    // Right wall collision
    if (CheckCollisionRecs(ballRect, rightWall) && ballVelocity.x > 0)
    {
        ballPosition.x = rightWall.x - ballSize;
        ballVelocity.x *= -1.1f;
        bumpCount++;
    }

    // Game over
    if (ballPosition.x > screenWidth || ballPosition.x < 0)
    {
        gameOver = true;
        ballVelocity = { 0, 0 };
    }
}

void Game::Draw() {
	BeginDrawing();
		ClearBackground(Color(20, 20, 20,255));
		DrawRectangleV(Vector2(100,leftWallY), Vector2(25, wallHeight), RAYWHITE);
		DrawRectangleV(Vector2(900,rightWallY), Vector2(25, wallHeight), RAYWHITE);

		DrawCircleV(ballPosition, ballSize, RED);
        DrawText((std::string("Bumps: ") + std::to_string(bumpCount)).c_str(), float(screenWidth) / 2 - 300, float(screenHeight) / 2 - 200, 20, RAYWHITE);

		if (gameOver) {
            DrawText("Game Over", float(screenWidth) / 2 - 100, float(screenHeight) / 2 - 20, 40, RAYWHITE);
			if (GuiButton(Rectangle{ float(screenWidth) / 2 - 100, float(screenHeight) / 2 + 40, 200, 50 }, "Restart")) {
				gameOver = false;
				ballPosition = Vector2{ 500, 300 };
				ballVelocity = Vector2{ 5, 5 };
				bumpCount = 0;
			}
		}

	EndDrawing();
}
