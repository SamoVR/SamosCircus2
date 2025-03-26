#include <iostream>
#include "Simulator.h"

int main()
{
    std::cout << "Program started!\n\n";

    try 
    {
        Simulator simulator;
        simulator.start();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
