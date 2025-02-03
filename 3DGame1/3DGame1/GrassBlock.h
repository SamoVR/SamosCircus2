#ifndef GRASSBLOCK_H
#define GRASSBLOCK_H

#include "Block.h"

class GrassBlock : public Block {
public:
    GrassBlock() : Block(BlockType::GRASS, 1.5f, true, { 48, 48, 48, 48, 32, 0 }) {}  // front,back,left,right,bottom,top
};

#endif
