#include <vector>
#include <raylib.h>

class Game
{
	public:
		Game();
		void Run();
		bool gameOver = false;

	private:
		void Update();
		void Draw();

		bool windowClose = false;

		const int screenWidth = 1000;
		const int screenHeight = 600;

		const int wallHeight = 200;

		int bumpCount = 0;

		Vector2 ballPosition = Vector2{ 500, 300 };
		Vector2 ballVelocity = Vector2{ 5, 5 };
		const int ballSize = 20;

		int leftWallY = 200;
		int rightWallY = 350;
};

