#include <iostream>
#include "Game.h"

int main()
{
	try{
		Game game;
		while (true)
		{
			game.Update();
		}
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An error occurred: " << ex.what() << std::endl;
	}
	return 0;
}