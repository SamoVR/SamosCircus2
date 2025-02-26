#ifndef BEDROCKBLOCK_H
#define BEDROCKBLOCK_H

#include "Block.h"

class BedrockBlock : public Block {
public:
    BedrockBlock() : Block(BlockType::BEDROCK, -1.0f, true, { 17, 17, 17, 17, 17, 17 }) {}
};

#endif
