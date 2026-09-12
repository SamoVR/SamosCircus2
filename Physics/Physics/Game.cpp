#include "Game.h"
#include <iostream>

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
	//left
	if (IsKeyDown(KEY_W) && leftWallY > 0) {
		leftWallY -= 5;
	}
	if (IsKeyDown(KEY_S) && leftWallY < screenHeight - wallHeight) {
		leftWallY += 5;
	}

	//right
	if (IsKeyDown(KEY_UP) && rightWallY > 0) {
		rightWallY -= 5;
	}
	if (IsKeyDown(KEY_DOWN) && rightWallY < screenHeight - wallHeight) {
		rightWallY += 5;
	}

	ballPosition.x += ballVelocity.x;
	ballPosition.y += ballVelocity.y;

	if(ballPosition.y < 0+ballSize || ballPosition.y > screenHeight-ballSize) {
		ballVelocity.y *= -1;
	}
	if(ballPosition.x < 0+ballSize || ballPosition.x > screenWidth-ballSize) {
		ballVelocity.x *= -1;
	}
	if (ballPosition.x == 100 + ballSize*2 && ballPosition.y > leftWallY && ballPosition.y < leftWallY + wallHeight) {
		ballVelocity.x *= -1;
	}
	if (ballPosition.x == 900 - ballSize && ballPosition.y > rightWallY && ballPosition.y < rightWallY + wallHeight) {
		ballVelocity.x *= -1;
	}
	
	if (ballPosition.x > 900 || ballPosition.x < 100) {
		gameOver = true;
		ballVelocity = Vector2{ 0, 0 };
	}

}

void Game::Draw() {
	BeginDrawing();
		ClearBackground(Color(20, 20, 20,255));
		DrawRectangleV(Vector2(100,leftWallY), Vector2(25, wallHeight), RAYWHITE);
		DrawRectangleV(Vector2(900,rightWallY), Vector2(25, wallHeight), RAYWHITE);

		DrawCircleV(ballPosition, ballSize, RED);

		if (gameOver) {
			DrawText("Game Over", float(screenWidth) / 2 - 100, float(screenHeight) / 2 - 20, 40, RAYWHITE);
			if (GuiButton(Rectangle{ float(screenWidth) / 2 - 100, float(screenHeight) / 2 + 40, 200, 50 }, "Restart")) {
				gameOver = false;
				ballPosition = Vector2{ 500, 300 };
				ballVelocity = Vector2{ 5, 5 };
			}
		}

	EndDrawing();
}
