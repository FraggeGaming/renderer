#pragma once
#include "System.h"
#include "../../Renderer/Batch.h"
#include "../../Components/MeshCapsule.h"
#include "../../Components/TransformComponent.h"

#include "../../Components/GPUMemoryHandle.h"
#include "../../ChunkManager.h"

class Renderer : public System{

    
    void TryRender(std::unordered_map<int, std::vector<int>>& meshGroups, TransformComponent& t, int meshID, GPUMemoryHandle& h);
    public:
    std::unique_ptr<Batch> batch;

    Renderer(Shader shader, VertexBufferLayout layout);

    void CreateBatch(Shader shader, VertexBufferLayout layout);
    void SetCamera(glm::mat4 CameraMatrix);

    GPUMemoryHandle LoadMesh(Entity id, MeshCapsule &m, glm::mat4 t);
    void UnloadMesh(Entity id);

    void SetProjection(glm::mat4 mat);

    

    void Start() override;
    void Update(float dt) override;
    void Render() override;
    void DebugTrace();
};