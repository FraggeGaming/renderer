#pragma once
#include <unordered_map>
#include <typeindex>
#include <vector>

#include "Component.h"

class ECS;



struct EntityHandle{
    int id = 0;

    bool operator==(const EntityHandle& other) const {
        return id == other.id;
    }
};

namespace std {
    template<>
    struct hash<EntityHandle> {
        size_t operator()(const EntityHandle& h) const noexcept {
            return std::hash<int>()(h.id);
        }
    };
}



struct Entity{
    
    static int handleCount;
    //Used to fetch the entity index from the entity array
    EntityHandle handle;
    ECS* ecs = nullptr;
    std::unordered_map<std::type_index, int> component_map;

    Entity() = default;

    Entity(ECS* ecs_ptr, int id)
        : handle(EntityHandle{id}), ecs(ecs_ptr) {}

    //Returns a copy of the handle
    EntityHandle GetHandle(){
        return handle;
    }

    void SetComponentIndex(std::type_index type, int index){
        component_map.at(type) = index;
    }

    template<typename T>
    void AddComponent(T& component);

    template<typename T>
    T& GetComponent();

    template <typename T>
    bool Has();

        
};


#include "ECS.h"

 // Forward component addition to ECS
template<typename T>
void Entity::AddComponent(T& component) {
    // CRITICAL: Call the ECS's templated AddComponent function, passing the correct type T.
    ecs->template AddComponent<T>(this->handle, component); 
}

template<typename T>
T& Entity::GetComponent() {
    // This is correct: Entity passes its handle to the ECS for lookup.
    return ecs->template GetComponent<T>(this->handle);
}

// template<typename T>
// T& Entity::GetComponent(EntityHandle handle) {
//     int index = component_map.at(std::type_index(typeid(T)));
//     return ecs->template GetComponent<T>(index);
// }

template <typename T>
bool Entity::Has(){
    return component_map.find(typeid(T)) != component_map.end();
}
