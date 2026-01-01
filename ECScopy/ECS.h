#pragma once
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

#include "Entity.h"
#include "ComponentStore.h"
#include "View.h"

class ECS{
    public:

    std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>> stores;

    std::unordered_map<EntityHandle, int> entity_mapper;
    std::vector<Entity> entities;

    int Entity_ID = 0;

    EntityHandle CreateEntity();

    template <typename T>
    void AddComponent(EntityHandle& handle, T& component){
        Entity& entity = GetEntity(handle);
        component.SetParent(entity_mapper.at(handle));

        std::type_index type (typeid(component));

        ComponentStore<T>& store = GetStore<T>();
        
        int index = store.Add(component);
        entity.component_map.insert({type, index});
    }

    template <typename...  Ts> //Pack of types
    IComponentStore* GetSmallestStore(){
        IComponentStore* smallest = nullptr;

        (([&]{
           auto& store = GetStore<Ts>();
           if(smallest == nullptr){
                smallest = &store;
           }

           else if(!smallest && store.GetStorageSize() < smallest->GetStorageSize()){
                smallest = &store;
           }
        }()), ...);

        return smallest;
    }



    Entity& GetEntity(EntityHandle& handle);

    /* Popswap index to remove */
    void EntityPopSwap(int idx);

    void Remove(EntityHandle& handle);

    template <typename T>
    ComponentStore<T>& GetStore(){
        std::type_index type = typeid(T);
        if (stores.find(type) == stores.end()) {
            stores[type] = std::make_unique<ComponentStore<T>>(); 
        }

        return static_cast<ComponentStore<T>&>(*stores.at(type));
    }

    template <typename T>
    T& GetComponent(EntityHandle& handle) {
        Entity& ent = GetEntity(handle);

        int store_idx = ent.component_map.at(typeid(T));

        ComponentStore<T>& store = GetStore<T>();
        
        return store.Get(store_idx);
    }

};