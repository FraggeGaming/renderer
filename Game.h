#pragma once
#include "Engine/ECS/Systems/System.h"
#include "Engine/Renderer/Batch.h"
#include "Engine/Components/MeshCapsule.h"
#include "Engine/Components/TransformComponent.h"

#include "Engine/Components/GPUMemoryHandle.h"

class Game : public System
{
    public: 

    void CreateController(Entity e_ptr);
    void Start() override;
    void Update(float dt) override;
    void Render() override;
};