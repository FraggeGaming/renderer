#include "RenderTypes.h"
#pragma once

using Entity = int;


class Renderer{

public:
    Renderer();

    ~Renderer();

    void Draw();

    void SetProjection(glm::mat4 mat);

    void CreateBatch(Shader shader, VertexBufferLayout layout);

    void SetCamera(glm::mat4 CameraMatrix);

    //void Remove(MeshComponent& e);

    void UpdateTransform(glm::mat4& t);


    //void HandleMeshChanges(const std::vector<std::unique_ptr<Entity>>& entities);
    
};