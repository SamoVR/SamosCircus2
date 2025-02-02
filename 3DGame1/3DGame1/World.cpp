#include "World.h"
#include <iostream>

World::World() {}

Chunk* World::getChunk(int x, int z) {
    auto key = std::make_pair(x, z);
    if (chunks.find(key) == chunks.end()) {
        chunks[key] = new Chunk(); // Generate a new chunk if not exist
    }
    return chunks[key];
}

void World::generateChunks(int radius) {
    for (int x = -radius; x <= radius; x++) {
        for (int z = -radius; z <= radius; z++) {
            getChunk(x, z);
        }
    }
    std::cout << "Generated " << (radius * 2 + 1) * (radius * 2 + 1) << " chunks." << std::endl;
}
