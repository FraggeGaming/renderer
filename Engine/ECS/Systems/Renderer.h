#pragma once
#include "System.h"
#include "../../Renderer/Batch.h"
#include "../../Components/MeshCapsule.h"
#include "../../Components/TransformComponent.h"

#include "../../Components/GPUMemoryHandle.h"
#include "../../ChunkManager.h"

class Renderer : public System{

    std::unique_ptr<Batch> batch;
    void TryRender(std::unordered_map<int, std::vector<int>>& meshGroups, TransformComponent& t, int meshID, GPUMemoryHandle& h);
    public:

    bool UseChunking = true;
    ChunkManager chunkMgr;

    Renderer(Shader shader, VertexBufferLayout layout);

    void CreateBatch(Shader shader, VertexBufferLayout layout);
    void SetCamera(glm::mat4 CameraMatrix);

    void Add(Entity id, MeshCapsule& m, glm::mat4 t);
    bool LoadMesh(Entity id, MeshCapsule &m, glm::mat4 t);
    void UnloadMesh(Entity id);

    void Remove(Entity id);

    void FetchChunk(TransformComponent& t, int xOffset, int yOffset, int zOffset);
    void SetProjection(glm::mat4 mat);

    void ClearUnseenChunks(){
        chunkMgr.UnloadAllUnseen(batch.get(), ecs);
    }
    

    void Start() override;
    void Update(float dt) override;
    void Render() override;
    void DebugTrace();
};