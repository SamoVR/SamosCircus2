#ifndef WORLD_H
#define WORLD_H

#include "Chunk.h"
#include <vector>
#include <unordered_map>

// Custom hash function for std::pair<int, int>
struct pair_hash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        return std::hash<T1>{}(p.first) ^ (std::hash<T2>{}(p.second) << 1);
    }
};

class World {
public:
    World();
    Chunk* getChunk(int x, int z);
    void generateChunks(int radius); // Load chunks around player

    // Use custom hash function
    std::unordered_map<std::pair<int, int>, Chunk*, pair_hash> chunks;

private:
};

#endif
