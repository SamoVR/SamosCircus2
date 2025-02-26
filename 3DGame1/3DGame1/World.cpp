#include "World.h"
#include "Block.h"
#include <iostream>
#include <unordered_map>

#define CHUNK_SIZE 16 // Define chunk size as 16x16 blocks like Minecraft


World::World() {}

Chunk* World::getChunk(int chunkX, int chunkZ) {
    auto key = std::make_pair(chunkX, chunkZ);
    if (chunks.find(key) == chunks.end()) {
        chunks[key] = new Chunk(chunkX, chunkZ); // Generate a new chunk if not exist
    }
    return chunks[key];
}

void World::generateChunks(int radius) {
    for (int x = -radius; x <= radius; x++) {
        for (int z = -radius; z <= radius; z++) {
            getChunk(x, z); // Ensure chunks exist
        }
    }
    std::cout << "Generated " << (radius * 2 + 1) * (radius * 2 + 1) << " chunks." << std::endl;
}

Chunk* World::getChunkAt(int worldX, int worldZ) {
    int chunkX = worldX / CHUNK_SIZE;
    int chunkZ = worldZ / CHUNK_SIZE;
    return getChunk(chunkX, chunkZ);
}

Block* World::getBlockAt(int worldX, int worldY, int worldZ) {
    Chunk* chunk = getChunkAt(worldX, worldZ);
    if (chunk) {
        return chunk->getBlock(worldX % CHUNK_SIZE, worldY, worldZ % CHUNK_SIZE);
    }
    return nullptr;
}

void World::placeBlock(int worldX, int worldY, int worldZ, BlockType type) {
    Chunk* chunk = getChunkAt(worldX, worldZ);
    if (chunk) {
        chunk->setBlock(worldX % CHUNK_SIZE, worldY, worldZ % CHUNK_SIZE, type);
    }
}
