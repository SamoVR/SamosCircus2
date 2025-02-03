#ifndef CHUNK_H
#define CHUNK_H

#include "Block.h"
#include "BlockFactory.h"

const int CHUNK_SIZE = 16;

class Chunk {
public:
    Block* blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // Pointer array

    Chunk();
    ~Chunk(); // Destructor to free memory
    void setBlock(int x, int y, int z, BlockType type);


private:
    void generateTerrain(); // Separate function for terrain logic
};

#endif
