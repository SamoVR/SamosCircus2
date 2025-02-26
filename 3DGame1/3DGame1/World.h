#ifndef WORLD_H
#define WORLD_H

#include "Chunk.h"
#include <unordered_map>

// Define chunk positioning grid (Minecraft-style)
inline int worldToChunkCoord(int coord) {
    return coord / CHUNK_SIZE - (coord < 0);
}

// Custom hash function for chunk coordinate pairs
struct ChunkCoordHash {
    std::size_t operator()(const std::pair<int, int>& p) const {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

class World {
public:
    World();

    void generateChunks(int radius);
    Chunk* getChunk(int chunkX, int chunkZ);
    Chunk* getChunkAt(int worldX, int worldZ);

    Block* getBlockAt(int x, int y, int z);
    void placeBlock(int x, int y, int z, BlockType type);
    std::unordered_map<std::pair<int, int>, Chunk*, ChunkCoordHash> chunks;

private:
    //
};

#endif
