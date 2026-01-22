#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_map>
//#include <mutex>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "SSBOBuffer.h"

#include "BufferHeap.h"

#include "VertexArray.h"
#include "RenderTypes.h"
#include "../Shader/Shader.h"
#include <memory>
#include "../Components/GPUMemoryHandle.h"
#include "../Components/MeshCapsule.h"
#include "../Engine.h"
//Forward declaration
struct AssetManager;

using Entity = int;
struct MeshGeometryInfo {
    unsigned int indexCount;
    unsigned int indexOffset;
    int vboOffset;
};

struct GPUData{
    GPUMemoryHandle handle;
    int meshID;
    glm::mat4 transform;
    MemoryBlock vBlock;
    MemoryBlock iBlock;
    MemoryBlock tBlock;
};

class Batch{
public:
    size_t vertexOffset = 0;
    size_t indexOffset = 0;

    // Sizes for GPU buffers (in bytes)
    size_t vertexBufferSize;
    size_t indexBufferSize;


    Shader shader;
    
    VertexArray va;
    VertexBuffer vb;
    IndexBuffer ib;
    SSBOBuffer transformBuffer; //Transform buffer binding 0 
    SSBOBuffer instanceLookupBuffer; //Binding 1

    //For gldrawindirect
    unsigned int indBuffer;

    std::vector<GPUMemoryHandle> drawCommands;

    std::unordered_map<int, MeshGeometryInfo> geometryRegistry;
    std::vector<GPUData> gpuData;

    //std::mutex _mutex;

    Batch(Shader shader, size_t vertexBufferBytes = 100ull * 1024ull * 1024ull, size_t indexBufferBytes = 50ull * 1024ull * 1024ull, size_t transformBufferSize = 64ull * 1024ull * 1024ull);
    ~Batch();

    void UpdateTransform(int idx, glm::mat4 t);
    void Unload(GPUMemoryHandle handle);
    void AddLayout(const VertexBufferLayout& layout);
    void Bind();
    void Draw();

    void UpdateInstanceLookupBuffer(const std::vector<int>& lookupTable);
    void SetDrawVector(const std::vector<GPUMemoryHandle>& commands);

    GPUMemoryHandle Load(MeshCapsule& m, Mesh& mesh, glm::mat4 t);
    GPUMemoryHandle Load(int meshID, Mesh& mesh, glm::mat4 t);

    GPUMemoryHandle CPULoad(int meshID, Mesh& mesh, glm::mat4 t);
    void GPULoad(AssetManager* assetManager);
    // Debug helper: prints details about a GPU memory handle and associated mesh
    void DebugPrintGPUMemoryHandle(const GPUMemoryHandle& handle, int meshID, const char* action);

private:
    GPUMemoryHandle LoadInternal(int meshId, Mesh& mesh, glm::mat4 t);

};