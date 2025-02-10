#include "Chunk.h"
#include "BlockFactory.h"
#include <iostream>

Chunk::Chunk() {
    generateTerrain(); // Call separate function for terrain
}

Chunk::~Chunk() {
    // Free memory to prevent leaks
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                delete blocks[x][y][z]; // Free each allocated block
            }
        }
    }
}

void Chunk::generateTerrain() {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                // Grass on the bottom, air above
                blocks[x][y][z] = (y == 0)
                    ? BlockFactory::createBlock(BlockType::GRASS)
                    : BlockFactory::createBlock(BlockType::AIR);
            }
        }
    }
}

Block* Chunk::getBlock(int x, int y, int z) {
    if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE) {
        return blocks[x][y][z]; // Return the block at the given coordinates
    }

    return nullptr; // Return nullptr if the coordinates are out of bounds
}

void Chunk::setBlock(int x, int y, int z, BlockType type) {
    std::cout << x;
    if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE) {
        delete blocks[x][y][z];  // Delete the old block to prevent memory leaks
        blocks[x][y][z] = BlockFactory::createBlock(type);
    }
}

