#include "Game.h"
#include "Ball.h"
#include <cmath>
#include <iostream>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

Game::Game() {
    InitWindow(screenWidth, screenHeight, "Ball vs Ball");
    SetTargetFPS(60);
    objects.push_back(&playerBall);
    objects.push_back(&enemyBall);
}

void Game::Run() {
    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ESCAPE)) break;

        Update();
        Draw();
    }
    CloseWindow();
}

void Game::Update() {
    for (Ball* ball : objects) {
        ball->position.x += ball->velocity.x;
        ball->position.y += ball->velocity.y;
    }

    for (Ball* ball : objects) {
        if (ball->position.x - ball->radius < 0) {
            ball->position.x = ball->radius;
            ball->velocity.x *= -1;
        }
        else if (ball->position.x + ball->radius > screenWidth) {
            ball->position.x = screenWidth - ball->radius;
            ball->velocity.x *= -1;
        }

        if (ball->position.y - ball->radius < 0) {
            ball->position.y = ball->radius;
            ball->velocity.y *= -1;
        }
        else if (ball->position.y + ball->radius > screenHeight) {
            ball->position.y = screenHeight - ball->radius;
            ball->velocity.y *= -1;
        }
    }

    for (size_t i = 0; i < objects.size(); i++) {
        for (size_t j = i + 1; j < objects.size(); j++) { 
            Ball* b1 = objects[i];
            Ball* b2 = objects[j];

            float dx = b2->position.x - b1->position.x;
            float dy = b2->position.y - b1->position.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < b1->radius + b2->radius) {
                b1->velocity.x *= -1;
                b1->velocity.y *= -1;
                b2->velocity.x *= -1;
                b2->velocity.y *= -1;

                float overlap = 0.5f * (b1->radius + b2->radius - distance);
                if (distance == 0) distance = 1.0f;

                b1->position.x -= overlap * (dx / distance);
                b1->position.y -= overlap * (dy / distance);
                b2->position.x += overlap * (dx / distance);
                b2->position.y += overlap * (dy / distance);
            }
        }
    }
}

void Game::Draw() {
    BeginDrawing();
    ClearBackground(Color{ 15, 15, 15, 255 });

    DrawCircle(playerBall.position.x, playerBall.position.y, playerBall.radius, RAYWHITE);
    DrawCircle(enemyBall.position.x, enemyBall.position.y, enemyBall.radius, RED);

    DrawText("YOU 100/100", playerBall.position.x - 50, playerBall.position.y + 50, 20, WHITE);
    DrawText("ENEMY 100/100", enemyBall.position.x - 50, enemyBall.position.y + 50, 20, WHITE);
    EndDrawing();
}