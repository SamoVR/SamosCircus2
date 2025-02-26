#ifndef CHUNK_H
#define CHUNK_H

#include "Block.h"
#include "BlockFactory.h"

const int CHUNK_SIZE = 16;
const int CHUNK_HEIGHT = 256; // Define a proper world height

class Chunk {
public:
    Block* blocks[CHUNK_SIZE][CHUNK_HEIGHT][CHUNK_SIZE]; // 3D grid of blocks

    int chunkX, chunkZ;  // Store chunk coordinates

    Chunk(int chunkX, int chunkZ);
    ~Chunk();

    void setBlock(int x, int y, int z, BlockType type);
    Block* getBlock(int x, int y, int z);

private:
    void generateTerrain();
};

#endif
