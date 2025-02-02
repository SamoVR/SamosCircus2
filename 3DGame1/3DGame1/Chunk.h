#ifndef CHUNK_H
#define CHUNK_H

#include "Block.h"

const int CHUNK_SIZE = 16;

class Chunk {
public:
    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE]; // 3D block array

    Chunk(); // Constructor
};

#endif
