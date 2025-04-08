#include "Engine.h"
#include <iostream>

int main()
{
    try {
        Engine engine;   // Create the Engine instance
        engine.run();  // Run the engine loop
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

	return 0;
}