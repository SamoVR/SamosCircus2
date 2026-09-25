#include "Game.h"
#include "Ball.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

Game::Game() {
	InitWindow(screenWidth, screenHeight, "Ball vs Ball");
	SetTargetFPS(60);
}

void Game::Run() {
	while (!WindowShouldClose()) {
		if (IsKeyPressed(KEY_ESCAPE)) break;

		Update();
		Draw();
	}
	CloseWindow();
}

void Game::handleCollision(Ball& ball) {
	objects.push_back(ball);
	if (ball.position.x - ball.radius < 0 || ball.position.x + ball.radius > screenWidth) {
		ball.velocity.x *= -1;
	}
	if (ball.position.y - ball.radius < 0 || ball.position.y + ball.radius > screenHeight) {
		ball.velocity.y *= -1;
	}
	
	for(int i = 0; i < objects.size(); i++) {
		Ball& otherBall = objects[i];
		if (&ball != &otherBall) {
			float dx = otherBall.position.x - ball.position.x;
			float dy = otherBall.position.y - ball.position.y;
			float distance = sqrt(dx * dx + dy * dy);
			std::cout << "Distance between balls: " << distance << std::endl;
			if (distance < ball.radius + otherBall.radius) {
				ball.velocity.x *= -1;
				ball.velocity.y *= -1;
			}
		}
	}
}

void Game::Update() {
	playerBall.position.x += playerBall.velocity.x;
	playerBall.position.y += playerBall.velocity.y;
	enemyBall.position.x += enemyBall.velocity.x;
	enemyBall.position.y += enemyBall.velocity.y;

	handleCollision(playerBall);
	handleCollision(enemyBall);

}

void Game::Draw() {
	BeginDrawing();
		ClearBackground(Color(15, 15, 15));
		DrawCircle(playerBall.position.x, playerBall.position.y, playerBall.radius, RAYWHITE);
		DrawCircle(enemyBall.position.x, enemyBall.position.y, enemyBall.radius, RED);
		DrawText("YOU ""100/100", playerBall.position.x - 50, playerBall.position.y + 50, 20, WHITE);
		DrawText("ENEMY ""100/100", enemyBall.position.x-50, enemyBall.position.y+50, 20, WHITE);
		//DrawRectangleLines(screenWidth/2 - 600, screenHeight/2 - 500, 1200, 1000, WHITE);
	EndDrawing();
}
