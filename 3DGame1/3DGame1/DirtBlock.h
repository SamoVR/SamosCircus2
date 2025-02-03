#ifndef DIRTBLOCK_H
#define DIRTBLOCK_H

#include "Block.h"

class DirtBlock : public Block {
public:
    DirtBlock() : Block(BlockType::DIRT, 1.0f, true,{ 32, 32, 32, 32, 32, 32 }) {}  // Texture ID 1 for dirt
};

#endif
