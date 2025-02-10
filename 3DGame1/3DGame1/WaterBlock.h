#ifndef WATERBLOCK_H
#define WATERBLOCK_H

#include "Block.h"

class WaterBlock : public Block {
public:
    WaterBlock() : Block(BlockType::WATER, 3.0f, false, { 253, 253, 253, 253, 253, 253 }) {}  // Texture ID 2 for stone
};

#endif
