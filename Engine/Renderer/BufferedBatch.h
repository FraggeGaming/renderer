#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_map>

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


using Entity = int;
struct MeshGeometryInfo {
    unsigned int indexCount;
    unsigned int indexOffset;
    int vboOffset;
};

class BufferedBatch{
public:
    size_t vertexOffset = 0;
    size_t indexOffset = 0;

    // Sizes for GPU buffers (in bytes). These are configured at construction time.
    size_t vertexBufferSize;
    size_t indexBufferSize;

    std::vector<glm::mat4> transforms;

    Shader shader;
    
    VertexArray va;
    VertexBuffer vb;
    IndexBuffer ib;
    SSBOBuffer ssboBuffer; //Transform buffer binding 0 

    //For gldrawindirect
    unsigned int indBuffer;
    unsigned int lookupBuffer = 0; // Binding 1: Instance Lookup Table

    std::vector<GPUMemoryHandle> drawCommands;

    std::unordered_map<int, MeshGeometryInfo> geometryRegistry;

    // Constructor: optional sizes (bytes). Defaults chosen as practical safe values.
    BufferedBatch(Shader shader, size_t vertexBufferBytes = 100ull * 1024ull * 1024ull, size_t indexBufferBytes = 50ull * 1024ull * 1024ull);
    ~BufferedBatch();


    int AddTransform(glm::mat4 t);
    void UpdateTransform(int idx, glm::mat4 t);
    void Unload(GPUMemoryHandle handle);
    void AddLayout(const VertexBufferLayout& layout);
    void Bind();
    void Draw();
    void ClearBufferedData();
    void UpdateCommandBuffer();

    void UpdateInstanceLookupBuffer(const std::vector<int>& lookupTable);
    void SetDrawVector(const std::vector<GPUMemoryHandle>& commands);

    GPUMemoryHandle Load(BufferedMesh& m, Mesh mesh, glm::mat4 t);
    // Debug helper: prints details about a GPU memory handle and associated mesh
    void DebugPrintGPUMemoryHandle(const GPUMemoryHandle& handle, int meshID, const char* action);

private:
    // Ensure there is enough GPU buffer capacity for the next upload
    void EnsureVertexCapacity(size_t neededBytes);
    void EnsureIndexCapacity(size_t neededBytes);
    void EnsureSSBOCapacity(size_t neededBytes);
    

    //TODO - this is just for prototyping, later use more efficient way
    //void ClearAndRebuild(const std::vector<std::unique_ptr<Entity>>& entities);

};