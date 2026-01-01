#pragma once
#include "System.h"
#include "../../Renderer/BufferedBatch.h"
#include "../../Components/BufferedMesh.h"
#include "../../Components/GPUMemoryHandle.h"

class BatchedRenderer : public System{

    std::unique_ptr<BufferedBatch> batch;
    public:

    BatchedRenderer(Shader shader, VertexBufferLayout layout);

    void CreateBatch(Shader shader, VertexBufferLayout layout);
    void SetCamera(glm::mat4 CameraMatrix);

    void LoadMesh(Entity id, BufferedMesh& m, glm::mat4 t);

    void Remove(BufferedMesh& e);

    void UpdateTransform(glm::mat4& t);

    void SetProjection(glm::mat4 mat);

    void Start() override;
    void Update(float dt) override;
    void Render() override;
};