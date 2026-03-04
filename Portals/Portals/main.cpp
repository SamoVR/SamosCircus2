#include <iostream>
#include "Game.h"

int main()
{
	try{
		Game game;
		game.run();
	}
	catch (const std::exception& ex)
	{
		std::cerr << "An error occurred: " << ex.what() << std::endl;
	}
	return 0;
}