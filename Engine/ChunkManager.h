#pragma once
#include <unordered_map>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include "Components/TransformComponent.h"

using Entity = int;
struct ChunkPos{
    glm::vec3 position;

    bool operator==(const ChunkPos& other) const {
        return position == other.position;
    }
};

namespace std {
    template <>
    struct hash<ChunkPos> {
        size_t operator()(const ChunkPos& p) const {
            // A simple bit-shifting combine hash
            size_t h1 = hash<int>{}(p.position.x);
            size_t h2 = hash<int>{}(p.position.y);
            size_t h3 = hash<int>{}(p.position.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2); 
        }
    };
}

struct ChunkManager{
    std::unordered_map<ChunkPos, std::vector<Entity>> chunks;
    float chunkSize = 10.0f;

    ChunkPos currentLoaded;

    void Load(TransformComponent& t){
        ChunkPos pos = {glm::floor(t.position / chunkSize) * chunkSize};
        currentLoaded = pos;
    }

    void Unload(TransformComponent& t){
        ChunkPos pos = {glm::floor(t.position / chunkSize) * chunkSize};
        chunks.erase(pos);

    }

    bool InCurrectChunk(TransformComponent& t){
        ChunkPos pos = {glm::floor(t.position / chunkSize) * chunkSize};
        return pos.position == currentLoaded.position;
    }

    void AddToChunk(TransformComponent& t, Entity e){
        ChunkPos pos = {glm::floor(t.position / chunkSize) * chunkSize};
        chunks[pos].push_back(e);
    }

    void RemoveFromChunk(TransformComponent& t, Entity e){
        ChunkPos pos = {glm::floor(t.position / chunkSize) * chunkSize};
        auto& vec = chunks[pos];
        vec.erase(std::remove(vec.begin(), vec.end(), e), vec.end());
    }
};

