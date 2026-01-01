#pragma once
#include <vector>
#include <tuple>
#include <algorithm>
#include "ComponentStore.h"

// Forward declarations 
class ECS;
using Entity = int;

template <typename... Ts>
class View {
    ECS& ecs;
    // Cache the pointers to the specific stores to avoid map lookups
    std::tuple<ComponentStore<Ts>*...> cachedStores;

public:
    View(ECS& ecs);

    //iterator for for each functionality
    struct Iterator {
        int index;
        const std::vector<int>& smallestEntityList;
        std::tuple<ComponentStore<Ts>*...>& stores;

        // Skip entities that dont have the full component mask
        void SkipInvalid() {
            while (index < (int)smallestEntityList.size() && !HasAll(smallestEntityList[index])) {
                index++;
            }
        }

        bool HasAll(int entityId) {
            //Direct pointer access via tuple
            return std::apply([entityId](auto... storePtrs) {
                return (storePtrs->Has(entityId) && ...);
            }, stores);
        }

        int operator*() const { return smallestEntityList[index]; }

        Iterator& operator++() {
            index++;
            SkipInvalid();
            return *this;
        }

        bool operator!=(const Iterator& other) const {
            return index != other.index;
        }
    };

    // Finds the store with the fewest entities
    IComponentStore* GetSmallest() {
        IComponentStore* smallest = nullptr;
        std::apply([&](auto... stores) {
            ((smallest = (!smallest || stores->GetStorageSize() < smallest->GetStorageSize()) 
                ? stores : smallest), ...);
        }, cachedStores);
        return smallest;
    }

    //fast functional iteration
    template <typename F>
    void each(F&& func) {
        IComponentStore* smallestBase = GetSmallest();
        if (!smallestBase) return;

        const std::vector<int>& entities = smallestBase->GetEntityIndices();

        for (int i = 0; i < (int)entities.size(); ++i) {
            int entityId = entities[i];

            if (HasAllInternal(entityId)) {
                // Pass direct references to the lambda
                func(entityId, std::get<ComponentStore<Ts>*>(cachedStores)->Get(entityId)...);
            }
        }
    }

    bool HasAllInternal(int entityId) {
        return std::apply([entityId](auto... storePtrs) {
            return (storePtrs->Has(entityId) && ...);
        }, cachedStores);
    }

    Iterator begin() {
        IComponentStore* smallest = GetSmallest();
        const std::vector<int>& list = smallest->GetEntityIndices();
        Iterator it{ 0, list, cachedStores };
        it.SkipInvalid();
        return it;
    }

    Iterator end() {
        IComponentStore* smallest = GetSmallest();
        const std::vector<int>& list = smallest->GetEntityIndices();
        return Iterator{ (int)list.size(), list, cachedStores };
    }
};