#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "SSBOBuffer.h"

#include "BufferHeap.h"

#include "VertexArray.h"
#include "RenderTypes.h"
#include "../Shader/Shader.h"
#include "../Components/BufferedMesh.h"

#include <memory>
#include "../Components/GPUMemoryHandle.h"




class BufferedBatch{
public:
    size_t vertexOffset = 0;
    size_t indexOffset = 0;

    const size_t vertexBufferSize = 10240 * 10240; //100 mb
    const size_t indexBufferSize = 10240 * 10240;

    std::vector<glm::mat4> transforms;

    Shader shader;
    
    VertexArray va;
    VertexBuffer vb;
    IndexBuffer ib;
    SSBOBuffer ssboBuffer; //Transform buffer

    //For gldrawindirect
    unsigned int indBuffer;

    std::vector<GPUMemoryHandle> drawCommands;

    //for ssbo
    unsigned int ssbo = 0;


    BufferedBatch(Shader shader);
    ~BufferedBatch();


    int AddTransform(glm::mat4 t);
    void UpdateTransform(int idx, glm::mat4 t);
    void AddMesh(Mesh mesh, int transformIndex);
    void Remove(int index);
    void AddLayout(const VertexBufferLayout& layout);
    void Bind();
    void Draw();
    void ClearBufferedData();
    void SetDrawVector(std::vector<GPUMemoryHandle> commands);
    void UpdateCommandBuffer();
    GPUMemoryHandle Load(BufferedMesh& m, glm::mat4 t);
    

    //TODO - this is just for prototyping, later use more efficient way
    //void ClearAndRebuild(const std::vector<std::unique_ptr<Entity>>& entities);

};