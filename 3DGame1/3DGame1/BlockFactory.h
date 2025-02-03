#ifndef BLOCKFACTORY_H
#define BLOCKFACTORY_H

#include "Block.h"

// BLOCKS //
#include "GrassBlock.h"
#include "DirtBlock.h"
#include "StoneBlock.h"
#include "BedrockBlock.h"

class BlockFactory {
public:
    static void init() {
        Block::loadTextureAtlas("assets/textures/terrain.png");  // Load the texture atlas
    }

    static Block* createBlock(BlockType type) {
        switch (type) {
        case BlockType::GRASS: return new GrassBlock();
        case BlockType::DIRT: return new DirtBlock();
        case BlockType::STONE: return new StoneBlock();
        case BlockType::BEDROCK: return new BedrockBlock();
        default: return nullptr;
        }
    }
};

#endif
