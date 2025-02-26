#include "Chunk.h"
#include "BlockFactory.h"
#include <iostream>

Chunk::Chunk(int chunkX, int chunkZ) : chunkX(chunkX), chunkZ(chunkZ) {
    // Initialize all blocks to nullptr before terrain generation
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                blocks[x][y][z] = nullptr;
            }
        }
    }

    generateTerrain();
}

Chunk::~Chunk() {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (blocks[x][y][z] != nullptr) {  // Prevent double deletion
                    delete blocks[x][y][z];
                    blocks[x][y][z] = nullptr;  // Nullify after deletion
                }
            }
        }
    }
}

void Chunk::generateTerrain() {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_HEIGHT; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (y == 0) {
                    blocks[x][y][z] = BlockFactory::createBlock(BlockType::GRASS);
                }
                else {
                    blocks[x][y][z] = BlockFactory::createBlock(BlockType::AIR);
                }
            }
        }
    }
}

Block* Chunk::getBlock(int localX, int localY, int localZ) {
    if (localX >= 0 && localX < CHUNK_SIZE &&
        localY >= 0 && localY < CHUNK_HEIGHT &&
        localZ >= 0 && localZ < CHUNK_SIZE) {
        return blocks[localX][localY][localZ];
    }
    return nullptr;
}

void Chunk::setBlock(int localX, int localY, int localZ, BlockType type) {
    if (localX >= 0 && localX < CHUNK_SIZE &&
        localY >= 0 && localY < CHUNK_HEIGHT &&
        localZ >= 0 && localZ < CHUNK_SIZE) {

        if (blocks[localX][localY][localZ] != nullptr) {
            delete blocks[localX][localY][localZ];  // Delete old block safely
        }
        blocks[localX][localY][localZ] = BlockFactory::createBlock(type);
    }
}
