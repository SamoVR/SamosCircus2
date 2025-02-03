#ifndef STONEBLOCK_H
#define STONEBLOCK_H

#include "Block.h"

class StoneBlock : public Block {
public:
    StoneBlock() : Block(BlockType::STONE, 3.0f, true, { 16, 16, 16, 16, 16, 16 }) {}  // Texture ID 2 for stone
};

#endif
