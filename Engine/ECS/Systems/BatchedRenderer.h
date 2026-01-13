#pragma once
#include "System.h"
#include "../../Renderer/BufferedBatch.h"
#include "../../Components/BufferedMesh.h"
#include "../../Components/TransformComponent.h"

#include "../../Components/GPUMemoryHandle.h"

class BatchedRenderer : public System{

    std::unique_ptr<BufferedBatch> batch;
    public:

    bool UseChunking = true;

    BatchedRenderer(Shader shader, VertexBufferLayout layout);

    void CreateBatch(Shader shader, VertexBufferLayout layout);
    void SetCamera(glm::mat4 CameraMatrix);

    void Add(Entity id, BufferedMesh& m, glm::mat4 t);
    bool LoadMesh(Entity id, BufferedMesh &m, glm::mat4 t);
    void UnloadMesh(Entity id);

    void Remove(Entity id);

    void FetchChunk(TransformComponent& t, int xOffset, int yOffset, int zOffset);
    void SetProjection(glm::mat4 mat);
    void TryRender(std::unordered_map<int, std::vector<int>>& meshGroups, TransformComponent& t, int meshID, GPUMemoryHandle& h);

    void Start() override;
    void Update(float dt) override;
    void Render() override;
    void DebugTrace();
};