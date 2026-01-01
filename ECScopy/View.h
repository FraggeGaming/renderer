#include "ECS.h"
#include "ComponentStore.h"

template<typename... Ts>
class View {
    ECS& ecs;
    std::vector<int> indices; // entity indices in ECS.entities

public:
    View(ECS& ecs) : ecs(ecs) {
        BuildIndexList();
    }

private:
    void BuildIndexList() {
        indices.clear();

        // Get the smallest store (by size)
        IComponentStore* smallestStore = ecs.template GetSmallestStore<Ts...>();
        int n = smallestStore->GetStorageSize();

        for (int i = 0; i < n; i++) {
            int entityIdx = smallestStore->GetParentIdx(i);
            Entity& e = ecs.entities[entityIdx];

            bool hasAll = ((e.Has<Ts>()) && ...); // fold expression
            if (hasAll)
                indices.push_back(entityIdx);
        }
    }

public:
    struct Iterator {
        ECS& ecs;
        std::vector<int>& indices;
        size_t i = 0;

        Iterator(ECS& ecs, std::vector<int>& idx, size_t start)
            : ecs(ecs), indices(idx), i(start) {}

        bool operator!=(const Iterator& other) const { return i != other.i; }
        Iterator& operator++() { ++i; return *this; }

        EntityHandle operator*() const {
            return ecs.entities[indices[i]].GetHandle();
        }
    };

    Iterator begin() { return Iterator{ecs, indices, 0}; }
    Iterator end()   { return Iterator{ecs, indices, indices.size()}; }
};

