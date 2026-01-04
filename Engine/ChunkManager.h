#pragma once
#include <unordered_map>
#include <vector>
#include <algorithm>

#include <glm/glm.hpp>
#include "Components/TransformComponent.h"

using Entity = int;
struct ChunkPos{
    int x, y, z;

    bool operator==(const ChunkPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

namespace std {
    template <>
    struct hash<ChunkPos> {
        size_t operator()(const ChunkPos& p) const {
            size_t h1 = hash<int>{}(p.x);
            size_t h2 = hash<int>{}(p.y);
            size_t h3 = hash<int>{}(p.z);
            return h1 ^ (h2 << 1) ^ (h3 << 2); 
        }
    };
}

struct ChunkManager{
    std::unordered_map<ChunkPos, std::vector<Entity>> chunks;
    float chunkSize = 10.0f;
    int loadRadius = 2;

    std::vector<ChunkPos> loadedChunks;


    //Load chunk based on transform position 
    void Load(TransformComponent& t){
        loadedChunks.clear();

        for(int x = -loadRadius; x <= loadRadius; x++){
            for(int y = -loadRadius; y <= loadRadius; y++){
                for(int z = -loadRadius; z <= loadRadius; z++){
                    ChunkPos pos = {
                        (int)glm::floor(t.position.x / chunkSize) + x,
                        (int)glm::floor(t.position.y / chunkSize) + y,
                        (int)glm::floor(t.position.z / chunkSize) + z
                    };
                    if(std::find(loadedChunks.begin(), loadedChunks.end(), pos) == loadedChunks.end()){
                        loadedChunks.push_back(pos);
                    }
                }
            }
        }
        
    }

    std::vector<Entity> GetAll(){
        std::vector<Entity> entities;
        for (const auto& [chunkPos, chunkEntities] : chunks) {
            entities.insert(entities.end(), chunkEntities.begin(), chunkEntities.end());
        }
        return entities;
    }

    std::vector<Entity> GetLoadedEntities(){
        std::vector<Entity> entities;
        for (const auto& chunkPos : loadedChunks) {
            auto& chunkEntities = chunks[chunkPos];
            entities.insert(entities.end(), chunkEntities.begin(), chunkEntities.end());
        }
        return entities;
    }


    //TODO
    bool Update(TransformComponent& t){
        //Update chunk if entity has moved to a new chunk
        ChunkPos currentPos = {
            (int)glm::floor(t.position.x / chunkSize),
            (int)glm::floor(t.position.y / chunkSize),
            (int)glm::floor(t.position.z / chunkSize)
        };

        
    }

    void AddToChunk(TransformComponent& t, Entity e){
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize),
            (int)glm::floor(t.position.y / chunkSize),
            (int)glm::floor(t.position.z / chunkSize)
        };
        chunks[pos].push_back(e);
    }

    void RemoveFromChunk(TransformComponent& t, Entity e){
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize),
            (int)glm::floor(t.position.y / chunkSize),
            (int)glm::floor(t.position.z / chunkSize)
        };
        auto& vec = chunks[pos];
        vec.erase(std::remove(vec.begin(), vec.end(), e), vec.end());
    }

    void Unload(TransformComponent& t){
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize),
            (int)glm::floor(t.position.y / chunkSize),
            (int)glm::floor(t.position.z / chunkSize)
        };
        for (auto it = loadedChunks.begin(); it != loadedChunks.end(); ++it) {
            if (*it == pos) {
                loadedChunks.erase(it);
                break;
            }
        }
    }
};

