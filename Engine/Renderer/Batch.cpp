#include "Batch.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Components/TransformComponent.h"

#include <iostream>
#include <algorithm>




Batch::Batch(Shader shader, size_t vertexBufferBytes, size_t indexBufferBytes, size_t transformBufferSize)
: vertexBufferSize(vertexBufferBytes), indexBufferSize(indexBufferBytes), vb((unsigned int)vertexBufferSize), ib((unsigned int)indexBufferSize), shader(shader)
{
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    //glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //indirect buffer
    glGenBuffers(1, &indBuffer);

    transformBuffer.SetBinding(0); // Binding 0 for transforms
    instanceLookupBuffer.SetBinding(1); // Binding 1
}

Batch::~Batch()
{
    transformBuffer.UnBind();
    instanceLookupBuffer.UnBind();
    vb.UnBind();
    ib.UnBind();
    glDeleteBuffers(1, &indBuffer);
}

void Batch::UpdateInstanceLookupBuffer(const std::vector<int>& lookupTable) {
    instanceLookupBuffer.AddData(0, lookupTable.size() * sizeof(int), lookupTable.data());
}

void Batch::SetDrawVector(const std::vector<GPUMemoryHandle>& commands) {
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER,
                commands.size() * sizeof(GPUMemoryHandle),
                commands.data(),
                GL_DYNAMIC_DRAW);
    drawCommands = commands;
}

void Batch::UpdateTransform(int idx, glm::mat4 t)
{
    transformBuffer.AddData(idx * sizeof(glm::mat4), sizeof(glm::mat4), (const void*)&t);
}

GPUMemoryHandle Batch::Load(MeshCapsule& m, Mesh& mesh, glm::mat4 t)  
{   
    int meshId = m.meshID; 

    return LoadInternal(meshId, mesh, t);
    
}

 GPUMemoryHandle Batch::Load(int meshID, Mesh& mesh, glm::mat4 t){
    return LoadInternal(meshID, mesh, t);
}

void Batch::GPULoad(AssetManager* assetManager){
    if(gpuData.size() == 0) return;
        
    for(size_t i = 0; i < gpuData.size(); i++){
        GPUData& d = gpuData[i];

        if(d.vBlock.size != 0 && d.iBlock.size != 0){
            Mesh& mesh = assetManager->Get(d.meshID);
            vb.LoadSubData(d.vBlock, mesh.vertices.data());
            ib.LoadSubData(d.iBlock, mesh.indices.data());
        }

        transformBuffer.LoadSubData(d.tBlock, &d.transform);
    }

    gpuData.clear();
}

//Reserve and load mesh without using gl calls.
GPUMemoryHandle Batch::CPULoad(int meshID, Mesh& mesh, glm::mat4 t){
    
    GPUData data = {};
    //If mesh is not registered
    if(geometryRegistry.find(meshID) == geometryRegistry.end()){
        size_t vBytes = sizeof(Vertex) * mesh.vertices.size();
        size_t iBytes = sizeof(unsigned int) * mesh.indices.size();

        MemoryBlock vblk = vb.Reserve(vBytes);
        MemoryBlock iblk = ib.Reserve(iBytes);

        data.vBlock = vblk;
        data.iBlock = iblk;
    
        if(vblk.size == 0 || iblk.size == 0){
            return GPUMemoryHandle{0,0,0,0,0, 0};
        }

        geometryRegistry[meshID] = {
            (unsigned int)mesh.indices.size(),
            (unsigned int)(iblk.offset / sizeof(unsigned int)),
            (int)(vblk.offset / sizeof(Vertex))
        };
    } 

    //Load the transform component
    MemoryBlock ssboBlock = transformBuffer.Reserve(sizeof(glm::mat4));
    data.tBlock = ssboBlock;

    if(ssboBlock.size == 0){
        return GPUMemoryHandle{0,0,0,0,0, 0};
    }

    data.meshID = meshID;
    data.transform = t;

    GPUMemoryHandle handle = {};
    MeshGeometryInfo& geo = geometryRegistry[meshID];
    
    handle.count = geo.indexCount;
    handle.indexOffset = geo.indexOffset;
    handle.vboOffset = geo.vboOffset;
    handle.ssboIndex = ssboBlock.offset / sizeof(glm::mat4);
    handle.instanceCount = 1;

    data.handle = handle;
    gpuData.push_back(data);
    
    // return data struct so we can load it on the render thread later
    return handle;
}

GPUMemoryHandle Batch::LoadInternal(int meshId, Mesh& mesh, glm::mat4 t)  
{   
    //For concurrency
    //std::lock_guard<std::mutex> lock(_mutex);

    if(geometryRegistry.find(meshId) == geometryRegistry.end()){
        size_t vBytes = sizeof(Vertex) * mesh.vertices.size();
        size_t iBytes = sizeof(unsigned int) * mesh.indices.size();

        MemoryBlock vBlock = vb.AddData(vBytes, (const void*)mesh.vertices.data());
        MemoryBlock iBlock = ib.AddData(iBytes, (const void*)mesh.indices.data());

        if(vBlock.size == 0 || iBlock.size == 0){
            //DebugPrintGPUMemoryHandle({}, meshId, "LoadFailed");
            return GPUMemoryHandle{0,0,0,0,0, 0};
        }

        geometryRegistry[meshId] = {
            (unsigned int)mesh.indices.size(),
            (unsigned int)(iBlock.offset / sizeof(unsigned int)),
            (int)(vBlock.offset / sizeof(Vertex))
        };
    }


    MemoryBlock ssboBlock = transformBuffer.AddData(sizeof(glm::mat4), &t);

    if(ssboBlock.size == 0){
        //DebugPrintGPUMemoryHandle({}, meshId, "LoadFailed");
        return GPUMemoryHandle{0,0,0,0,0, 0};
    }

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
    
}




void Batch::Unload(GPUMemoryHandle handle)
{
    //std::lock_guard<std::mutex> lock(_mutex);
    // Only free transform data
    transformBuffer.Free(handle.ssboIndex * sizeof(glm::mat4));
}

void Batch::AddLayout(const VertexBufferLayout &layout)
{
    va.AddBuffer(vb, layout);
}

void Batch::Bind()
{
    va.Bind();
    vb.Bind();
    ib.Bind();

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indBuffer);

    transformBuffer.Bind();
    instanceLookupBuffer.Bind();

}

void Batch::Draw()
{
    glMultiDrawElementsIndirect(
    GL_TRIANGLES, 
    GL_UNSIGNED_INT, 
    nullptr, 
    drawCommands.size(), 
    sizeof(GPUMemoryHandle));

}

void Batch::DebugPrintGPUMemoryHandle(const GPUMemoryHandle& handle, int meshID, const char* action)
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

    // Print mesh geometry info
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