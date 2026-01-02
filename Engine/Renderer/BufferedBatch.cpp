#include "BufferedBatch.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Components/MeshComponent.h"
#include "../Components/TransformComponent.h"

#include <iostream>
#include <algorithm>


BufferedBatch::BufferedBatch(Shader shader, size_t vertexBufferBytes, size_t indexBufferBytes)
: vertexBufferSize(vertexBufferBytes), indexBufferSize(indexBufferBytes), vb(nullptr, (unsigned int)vertexBufferSize), ib(nullptr, (unsigned int)indexBufferSize), shader(shader)
{
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //indirect buffer
    glGenBuffers(1, &indBuffer);

    //For lookuptable
    glGenBuffers(1, &lookupBuffer);

    //ssbo for passing transforms to the shader
    //glGenBuffers(1, &ssbo);
    
}

BufferedBatch::~BufferedBatch()
{
    glDeleteBuffers(1, &indBuffer);
    glDeleteBuffers(1, &lookupBuffer);
}

int BufferedBatch::AddTransform(glm::mat4 t)  
{
    int size = transforms.size();
    transforms.push_back(t);

    return size;
}

void BufferedBatch::UpdateInstanceLookupBuffer(const std::vector<int>& lookupTable) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, lookupBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, lookupTable.size() * sizeof(int), lookupTable.data(), GL_DYNAMIC_DRAW);
}

void BufferedBatch::SetDrawVector(const std::vector<GPUMemoryHandle>& commands) {
    drawCommands = commands;
    UpdateCommandBuffer();
}

void BufferedBatch::UpdateTransform(int idx, glm::mat4 t)
{
    ssboBuffer.OverWrite(idx * sizeof(glm::mat4), sizeof(glm::mat4), (const void*)&t);
}

GPUMemoryHandle BufferedBatch::Load(BufferedMesh& m, Mesh mesh, glm::mat4 t)  
{   
    int meshId = m.meshID; 

    if(geometryRegistry.find(meshId) == geometryRegistry.end()){
        size_t vBytes = sizeof(Vertex) * mesh.vertices.size();
        size_t iBytes = sizeof(unsigned int) * mesh.indices.size();

        EnsureVertexCapacity(vBytes);
        EnsureIndexCapacity(iBytes);

        MemoryBlock& vBlock = vb.AddData(vBytes, (const void*)mesh.vertices.data());
        MemoryBlock& iBlock = ib.AddData(iBytes, (const void*)mesh.indices.data());

        geometryRegistry[meshId] = {
            (unsigned int)mesh.indices.size(),
            (unsigned int)(iBlock.offset / sizeof(unsigned int)),
            (int)(vBlock.offset / sizeof(Vertex))
        };
    }

    // Ensure SSBO has space for a new transform
    EnsureSSBOCapacity(sizeof(glm::mat4));
    MemoryBlock& ssboBlock = ssboBuffer.AddData(sizeof(glm::mat4), &t);

    GPUMemoryHandle handle = {};
    MeshGeometryInfo& geo = geometryRegistry[meshId];
    
    handle.count = geo.indexCount;
    handle.indexOffset = geo.indexOffset;
    handle.vboOffset = geo.vboOffset;
    handle.ssboIndex = ssboBlock.offset / sizeof(glm::mat4);
    handle.instanceCount = 1;

    // Debug print for newly loaded mesh
    //DebugPrintGPUMemoryHandle(handle, meshId, "Load");

    return handle;
    /*
    Mesh& mesh = m.MeshComponent;

    MemoryBlock& vBlock = vb.AddData(sizeof(Vertex) * mesh.vertices.size(), (const void*)mesh.vertices.data());
    MemoryBlock& iBlock = ib.AddData(sizeof(unsigned int) * mesh.indices.size(), (const void*)mesh.indices.data());
    MemoryBlock& ssboBlock = ssboBuffer.AddData(sizeof(glm::mat4), (const void*)&t);

    GPUMemoryHandle cmd = GPUMemoryHandle();
    cmd.count = mesh.indices.size();
    cmd.instanceCount = 1;
    cmd.indexOffset = iBlock.offset / sizeof(unsigned int);
    cmd.vboOffset = vBlock.offset / sizeof(Vertex);
    cmd.baseInstance = 0;
    cmd.ssboIndex = ssboBlock.offset / sizeof(glm::mat4);

    std::cout << "\n--- Loaded Mesh ---\n";
    return cmd;
    */
    
}



void BufferedBatch::Remove(int index)
{
    if(index < 0 || index >= (int)drawCommands.size()){
        std::cout << "BufferedBatch::Remove - invalid index: " << index << std::endl;
        return;
    }

    // Print info about the command being removed
    GPUMemoryHandle removed = drawCommands[index];
    DebugPrintGPUMemoryHandle(removed, -1, "Remove");

    drawCommands.erase(drawCommands.begin() + index);
    transforms.erase(transforms.begin() + index);

}

void BufferedBatch::AddLayout(const VertexBufferLayout &layout)
{
    va.AddBuffer(vb, layout);
}

void BufferedBatch::Bind()
{
    va.Bind();
    vb.Bind();
    ib.Bind();

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indBuffer);

    //bind ssbo for transforms

    ssboBuffer.Bind();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, lookupBuffer);      // Lookup Table

    //UpdateCommandBuffer();

}

void BufferedBatch::Draw()
{
    //glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);

    glMultiDrawElementsIndirect(
    GL_TRIANGLES, 
    GL_UNSIGNED_INT, 
    nullptr, 
    drawCommands.size(), 
    sizeof(GPUMemoryHandle));

}



void BufferedBatch::UpdateCommandBuffer()
{
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER,
                 drawCommands.size() * sizeof(GPUMemoryHandle),
                 drawCommands.data(),
                 GL_DYNAMIC_DRAW);
}


void BufferedBatch::DebugPrintGPUMemoryHandle(const GPUMemoryHandle& handle, int meshID, const char* action)
{
    std::cout << "[BufferedBatch::" << action << "] ";
    if(meshID >= 0) std::cout << "MeshID: " << meshID << " ";
    else std::cout << "MeshID: (unknown) ";

    std::cout << "count: " << handle.count
              << " instanceCount: " << handle.instanceCount
              << " indexOffset: " << handle.indexOffset
              << " vboOffset: " << handle.vboOffset
              << " baseInstance: " << handle.baseInstance
              << " ssboIndex: " << handle.ssboIndex;

    // Print registry / buffer summary
    size_t registeredGeometries = geometryRegistry.size();
    size_t drawCmds = drawCommands.size();
    // Sum instances from current drawCommands
    size_t totalInstances = 0;
    for (const auto &c : drawCommands) totalInstances += c.instanceCount;

    std::cout << " | registeredGeometries: " << registeredGeometries
              << " drawCommands: " << drawCmds
              << " totalInstances: " << totalInstances
              << " vertexOffset(bytes): " << vertexOffset
              << " indexOffset(bytes): " << indexOffset
              << std::endl;

    // If we have geometry info for the mesh, print it
    if (meshID >= 0) {
        auto it = geometryRegistry.find(meshID);
        if (it != geometryRegistry.end()){
            const MeshGeometryInfo &g = it->second;
            std::cout << "   GeometryInfo -> indexCount: " << g.indexCount
                      << " indexOffset(elements): " << g.indexOffset
                      << " vboOffset(vertices): " << g.vboOffset
                      << std::endl;
        }
    }
}


void BufferedBatch::EnsureVertexCapacity(size_t neededBytes)
{
    if (vertexOffset + neededBytes <= vertexBufferSize) return;

    size_t newSize = std::max(vertexBufferSize * 2, vertexOffset + neededBytes);
    std::cout << "BufferedBatch: Growing vertex buffer " << vertexBufferSize << " -> " << newSize << " bytes" << std::endl;
    vb.Resize(newSize);
    vertexBufferSize = newSize;
}

void BufferedBatch::EnsureIndexCapacity(size_t neededBytes)
{
    if (indexOffset + neededBytes <= indexBufferSize) return;

    size_t newSize = std::max(indexBufferSize * 2, indexOffset + neededBytes);
    std::cout << "BufferedBatch: Growing index buffer " << indexBufferSize << " -> " << newSize << " bytes" << std::endl;
    ib.Resize(newSize);
    indexBufferSize = newSize;
}

void BufferedBatch::EnsureSSBOCapacity(size_t neededBytes)
{
    size_t used = transforms.size() * sizeof(glm::mat4);
    if (used + neededBytes <= ssboBuffer.GetSize()) return;

    size_t newSize = std::max(ssboBuffer.GetSize() * 2, used + neededBytes);
    if (newSize == 0) newSize = 1024 * 1024; // fallback
    std::cout << "BufferedBatch: Growing SSBO buffer " << ssboBuffer.GetSize() << " -> " << newSize << " bytes" << std::endl;
    ssboBuffer.Resize(newSize);
}
