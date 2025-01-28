#include "Game.h"

int main() {
    try {
        Game game;   // Create the Game instance
        game.run();  // Run the game loop
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
