
#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>

#include "Component.h"


struct IComponentStore { 
    virtual ~IComponentStore() = default; 
    virtual void PopSwapRemove(int idx) = 0;
    virtual int GetStorageSize() = 0;
    virtual int GetParentIdx(int idx) = 0;
    virtual void SetParentIdx(int idx, int new_idx) = 0;
};

template <typename T>
struct ComponentStore : public IComponentStore{
    std::vector<T> storage;
    std::vector<int> entityIndices; 

    int Add(T t, int entityIdx){
        storage.push_back(t);
        entityIndices.push_back(entityIdx;)
        return storage.size() - 1;
    }

    int GetStorageSize() override {
        return storage.size();
    }

    int GetParentIdx(int idx) override {
        return storage.at(idx).parent_idx;
    }

    void SetParentIdx(int idx, int new_idx) override {
        storage.at(idx).parent_idx = new_idx;
    }

    /*  Takes the index to be removed   */
    void PopSwapRemove(int idx) override{

        if(idx != storage.size()-1){
            storage.at(idx) = storage.back(); 
        }

        storage.pop_back();

        if(idx != entityIndices.size()-1){
            entityIndices.at(idx) = entityIndices.back(); 
        }

        entityIndices.pop_back();
    }

    T& Get(int idx){
        return storage.at(idx);
    }
};