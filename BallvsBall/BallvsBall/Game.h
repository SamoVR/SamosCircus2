#include <raylib.h>
#include <iostream>
#include <vector>
#include "Ball.h"

class Game
{
public:
	Game();
	void Run();
	std::vector<Ball> objects;

private:
	void Update();
	void Draw();
	void handleCollision(Ball &ball);

	const int screenWidth = 1800;
	const int screenHeight = 1200;

	Ball playerBall{ Vector2{static_cast<float>(screenWidth) / 4, static_cast<float>(screenHeight / 2)}, Vector2{10, 1}, 50 };
	Ball enemyBall{ Vector2{static_cast<float>(screenWidth) * 3 / 4, static_cast<float>(screenHeight / 2)}, Vector2{-10, -1}, 50 };

	enum ballType {spider, charge, bomb, spike, zone};


};

