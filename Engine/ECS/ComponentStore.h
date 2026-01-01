
#pragma once
#include <vector>
#include <unordered_map>
#include <typeindex>



struct IComponentStore { 
    virtual ~IComponentStore() = default; 
    virtual void Remove(int idx) = 0;
    virtual int GetStorageSize() = 0;
    virtual bool Has(int idx) const = 0;
    virtual const std::vector<int>& GetEntityIndices() const = 0;
};

template <typename T>
struct ComponentStore : public IComponentStore{
    std::vector<T> storage;
    std::vector<int> entityIndices; 
    std::vector<int> sparseMap;    // Sparse: Entity ID -> Storage Index


    static constexpr int NULL_INDEX = -1;

    ComponentStore(){
        //Preallocate for 10k entities
        sparseMap.reserve(10000); 
        entityIndices.reserve(10000);
        storage.reserve(10000);
    }

    int Add(T t, int entityIdx){
        if(entityIdx >= sparseMap.size()) {
            sparseMap.resize(entityIdx + 1, NULL_INDEX);
        }
        storage.push_back(t);
        entityIndices.push_back(entityIdx);
        sparseMap[entityIdx] = storage.size() - 1;
        return storage.size() - 1;
    }

    int GetStorageSize() override {
        return storage.size();
    }

    const std::vector<int>& GetEntityIndices() const override {
        return entityIndices;
    }

    /*  Takes the index to be removed   */
    void Remove(int entityIdx) override{
        if(!Has(entityIdx)){
            return;
        }

        int indexToRemove = sparseMap[entityIdx];

        int last = storage.size()-1;

        if(indexToRemove != last){
            // Move last data to the removed slot
            storage[indexToRemove] = std::move(storage.back());

            // Move the entity index to the removed slot
            int movedEntityId = entityIndices.back();
            entityIndices[indexToRemove] = movedEntityId;

            // Update sparsemap for the moved entity
            sparseMap[movedEntityId] = indexToRemove;
        }

        storage.pop_back();
        entityIndices.pop_back();
        sparseMap[entityIdx] = NULL_INDEX;
    }

    T& Get(int entityIdx){
        return storage[sparseMap[entityIdx]];
    }

    bool Has(int entityIdx) const {
        return entityIdx < sparseMap.size() && sparseMap[entityIdx] != NULL_INDEX;    
    }

    int Size() const { return storage.size(); }

    
};