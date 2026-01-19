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

    ChunkPos Set(TransformComponent& t, float divider){
        
        x = (int)glm::floor(t.position.x / divider);
        y = (int)glm::floor(t.position.y / divider);
        z = (int)glm::floor(t.position.z / divider);
        return *this;
    }

    std::string toString() const {
        return "(" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")";
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

    void Clear(){
        
        
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