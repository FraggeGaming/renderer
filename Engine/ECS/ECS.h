#pragma once
#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>

#include "ComponentStore.h"

using Entity = int;

template <typename... Ts> class View;

class ECS{
    public:
    int nextEntityId = 0;

    std::unordered_map<std::type_index, std::unique_ptr<IComponentStore>> stores;

    //std::vector<Entity> entities;

    ECS() = default;

    Entity CreateEntity(){
        
        return Entity{nextEntityId++};
    }

    template <typename T>
    void AddComponent(Entity entity, T component){
        auto& store = GetStore<T>();
        store.Add(component, entity);
    }

    template <typename T>
    T& GetComponent(int entity) {
        auto& store = GetStore<T>();
        return store.Get(entity);
    }

    template<typename T>
    bool HasComponent(int e) {
        auto& store = GetStore<T>();
        return store.Has(e);
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


    template <typename... Ts>
    View<Ts...> view();

    void Remove(int entityId){

        for(auto& [type, store] : stores) {
            if(store->Has(entityId))
                store->Remove(entityId);
        }
    }

    template <typename T>
    void RemoveComponent(int entityId){
        auto& store = GetStore<T>();
        
        store.Remove(entityId);
        
    }

    template <typename T>
    ComponentStore<T>& GetStore(){
        std::type_index type = typeid(T);
        if (stores.find(type) == stores.end()) {
            stores[type] = std::make_unique<ComponentStore<T>>(); 
        }

        return static_cast<ComponentStore<T>&>(*stores.at(type));
    }

};


#include "View.h"

//Define the view class
template <typename... Ts>
View<Ts...>::View(ECS& ecs) 
    : ecs(ecs), cachedStores(&ecs.GetStore<Ts>()...) 
{
}

//define the member function
template <typename... Ts>
View<Ts...> ECS::view() {
    return View<Ts...>(*this);
}