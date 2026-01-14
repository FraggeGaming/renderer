#pragma once
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <list>
#include <glm/glm.hpp>
#include "Components/TransformComponent.h"
#include "Components/GPUMemoryHandle.h"

template <typename T>
struct FreshQueue{
    std::list<T> data;
    std::unordered_map<T, typename std::list<T>::iterator> lookup;


    void push(const T& value){
        auto it = lookup.find(value);

        if(it != lookup.end()){
            data.splice(data.end(), data, it->second);
        }

        else{
            data.push_back(value);
            lookup[value] = std::prev(data.end());
        }
    }

    bool popIfExist(const T& value){
        auto it = lookup.find(value);

        if (it != lookup.end()){
            data.erase(it->second);
            lookup.erase(it);
            return true;
        }

        return false;
    }

    void pop(){
        if(data.empty()) return;
        lookup.erase(data.front());
        data.pop_front();
    }

    T peek(){
        if(data.empty()) return T{};
        return data.front();
    }
};

using Entity = int;
struct ChunkPos{
    int x, y, z;

    bool operator==(const ChunkPos& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    bool operator!=(const ChunkPos& other) const {
        return x != other.x || y != other.y || z != other.z;
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

struct EntityDrawInfo{
    Entity entity;
    GPUMemoryHandle handle;
    int MeshID;
};

struct Chunk{
    std::vector<Entity> entities;
    std::vector<EntityDrawInfo> gpuHandles;
    std::unordered_map<Entity, size_t> entityToHandleIndex;
    bool isLoaded = false;
    bool isVisible = false;

    public:
    void Add(Entity e){
        if(std::find(entities.begin(), entities.end(), e) == entities.end()){
            entities.push_back(e); 
        }
    }


    void UnLoad(Batch* batch, ECS* ecs){
        if(!isLoaded) {
            std::cout << "UnLoad skipped - chunk not loaded" << std::endl;
            return;
        }
        
        std::cout << "Unloading chunk with " << entities.size() << " entities" << std::endl;
        for(Entity e : entities){
            EntityDrawInfo drawinfo = gpuHandles[entityToHandleIndex[e]];
            GPUMemoryHandle handle = drawinfo.handle;
            batch->Unload(handle);
        }
        gpuHandles.clear();
        entityToHandleIndex.clear();
        isLoaded = false;
        isVisible = false;

        std::cout << "Cleared the chunk of data" << std::endl;
    }

    void LoadHandle(Entity e, GPUMemoryHandle newHandle, int meshID){

        if(entityToHandleIndex.find(e) != entityToHandleIndex.end()){
            size_t index = entityToHandleIndex[e];
            gpuHandles[index] = {e, newHandle, meshID};
        }

        else{
            gpuHandles.push_back({e, newHandle, meshID});
            entityToHandleIndex[e] = gpuHandles.size() - 1;
        }
    }

    GPUMemoryHandle Remove(Entity e){
        entities.erase(std::remove(entities.begin(), entities.end(), e), entities.end());
        GPUMemoryHandle handle = gpuHandles[entityToHandleIndex[e]].handle;
        gpuHandles.erase(gpuHandles.begin() + entityToHandleIndex[e]);
        entityToHandleIndex.erase(e);
        return handle;
    }

};

struct ChunkManager{
    std::unordered_map<ChunkPos, Chunk> chunks;
    float chunkSize = 16.0f;
    int loadRadius = 2;

    FreshQueue<ChunkPos> loadOrderQueue;
    std::vector<ChunkPos> activeChunks;

    Chunk& Get(TransformComponent& t, int xOffset = 0, int yOffset = 0, int zOffset = 0){
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize) + xOffset,
            (int)glm::floor(t.position.y / chunkSize) + yOffset,
            (int)glm::floor(t.position.z / chunkSize) + zOffset
        };
        return chunks[pos];
    }

    void Load(TransformComponent& t, int xOffset = 0, int yOffset = 0, int zOffset = 0){
        
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize) + xOffset,
            (int)glm::floor(t.position.y / chunkSize) + yOffset,
            (int)glm::floor(t.position.z / chunkSize) + zOffset
        };
        
        Chunk& chunk = chunks[pos];
        loadOrderQueue.push(pos);


        if(chunk.isLoaded)
            return;

        
        activeChunks.push_back(pos);
        chunk.isLoaded = true;
        chunk.isVisible = true;
    }


    void Unload(TransformComponent& t, int xOffset = 0, int yOffset = 0, int zOffset = 0){
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize) + xOffset,
            (int)glm::floor(t.position.y / chunkSize) + yOffset,
            (int)glm::floor(t.position.z / chunkSize) + zOffset
        };
        
        Chunk& chunk = chunks[pos];
        loadOrderQueue.popIfExist(pos);
        activeChunks.erase(std::remove(activeChunks.begin(), activeChunks.end(), pos), activeChunks.end());
        chunk.isLoaded = false;
        chunk.isVisible = false;
    }

    void Unload(ChunkPos pos){
        Chunk& chunk = chunks[pos];
        loadOrderQueue.popIfExist(pos);
        activeChunks.erase(std::remove(activeChunks.begin(), activeChunks.end(), pos), activeChunks.end());
        chunk.isLoaded = false;
        chunk.isVisible = false;
    }

    void UnloadOldestChunk(Batch* batch, ECS* ecs){
        if(loadOrderQueue.data.empty()) {
            std::cout << "UnloadOldestChunk: No chunks to unload!" << std::endl;
            return;
        }
        
        ChunkPos oldest = loadOrderQueue.peek();
        
        chunks[oldest].UnLoad(batch, ecs);  //Unload on the GPU
        Unload(oldest);  // Then mark as unloaded
        loadOrderQueue.pop();  // Remove from queue
    }

    void UnloadAllUnseen(Batch* batch, ECS* ecs){
        std::cout << "UnloadAllUnseen called - queue size: " << loadOrderQueue.data.size() 
                  << ", active chunks: " << activeChunks.size() << std::endl;
        
        std::vector<ChunkPos> toRemove;
        
        for (ChunkPos p : loadOrderQueue.data)
        {
            Chunk& chunk = chunks[p];
            
            // Unload chunks that are out of range
            if(chunk.isLoaded && !chunk.isVisible){
                //std::cout << "Unloading unseen chunk at (" << p.x << ", " << p.y << ", " << p.z << ")" << std::endl;
                chunk.UnLoad(batch, ecs);  //Unload on the GPU
                Unload(p);  // Mark the chunkpos as unloaded
                toRemove.push_back(p);
            }
        }
        
        std::cout << "Marked " << toRemove.size() << " chunks for removal" << std::endl;
        
        // Remove from loadOrderQueue
        for(ChunkPos p : toRemove){
            loadOrderQueue.popIfExist(p);
        }

        std::cout << "activechunks after cleanup:  " << activeChunks.size() << " chunks" << std::endl;
    }

    void ValidateEntityLocation(TransformComponent& t, int meshID, Entity e, ChunkPos oldPos){
        ChunkPos currentPos = {
            (int)glm::floor(t.position.x / chunkSize),
            (int)glm::floor(t.position.y / chunkSize),
            (int)glm::floor(t.position.z / chunkSize)
        };

        if(currentPos == oldPos) return;

        std::cout << "ChunkManager: Moving entity " << e << " from chunk (" 
                  << oldPos.x << ", " << oldPos.y << ", " << oldPos.z << ") to chunk ("
                  << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")" << std::endl;
        GPUMemoryHandle handle = chunks[oldPos].Remove(e);

        chunks[currentPos].Add(e);
        chunks[currentPos].LoadHandle(e, handle, meshID);
    }



    void AddToChunk(TransformComponent& t, Entity e){
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize),
            (int)glm::floor(t.position.y / chunkSize),
            (int)glm::floor(t.position.z / chunkSize)
        };
        chunks[pos].Add(e);
    }

    void RemoveFromChunk(TransformComponent& t, Entity e){
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize),
            (int)glm::floor(t.position.y / chunkSize),
            (int)glm::floor(t.position.z / chunkSize)
        };
        chunks[pos].Remove(e);
        
    }
};

