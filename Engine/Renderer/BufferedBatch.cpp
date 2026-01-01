#include "BufferedBatch.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Components/MeshComponent.h"
#include "../Components/TransformComponent.h"


BufferedBatch::BufferedBatch(Shader shader)
:vb(nullptr, vertexBufferSize), ib(nullptr, indexBufferSize), shader(shader)
{
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    //indirect buffer
    glGenBuffers(1, &indBuffer);

    //ssbo for passing transforms to the shader
    glGenBuffers(1, &ssbo);
    
}

BufferedBatch::~BufferedBatch()
{
}

int BufferedBatch::AddTransform(glm::mat4 t)  
{
    int size = transforms.size();
    transforms.push_back(t);

    return size;
}

void BufferedBatch::UpdateTransform(int idx, glm::mat4 t)
{

    transforms[idx] = t;
}

GPUMemoryHandle BufferedBatch::Load(BufferedMesh& m, glm::mat4 t)  
{
    // int size = transforms.size();
    // transforms.push_back(t);
    
    Mesh& mesh = m.MeshComponent;

    MemoryBlock& vBlock = vb.AddData(sizeof(Vertex) * mesh.vertices.size(), (const void*)mesh.vertices.data());
    MemoryBlock& iBlock = ib.AddData(sizeof(unsigned int) * mesh.indices.size(), (const void*)mesh.indices.data());
    MemoryBlock& ssboBlock = ssboBuffer.AddData(sizeof(glm::mat4), (const void*)&t);
    
    //Also add the ssbo heap -> SSBOBuffer
    //vb.AddData(vBlock.offset, sizeof(Vertex) * mesh.vertices.size(), (const void*)mesh.vertices.data());
    //ib.AddData(iBlock.offset, sizeof(unsigned int) * mesh.indices.size(), (const void*)mesh.indices.data());


    GPUMemoryHandle cmd = GPUMemoryHandle();
    cmd.count = mesh.indices.size();
    cmd.instanceCount = 1;
    cmd.indexOffset = iBlock.offset / sizeof(unsigned int);
    cmd.vboOffset = vBlock.offset / sizeof(Vertex);
    cmd.baseInstance = 0;
    cmd.ssboIndex = ssboBlock.offset / sizeof(glm::mat4);

    std::cout << "\n--- Loaded Mesh ---\n";

    return cmd;
}

void BufferedBatch::AddMesh(Mesh mesh, int transformIndex)
{
    



    //unsigned int instanceIndex = commands.size();

    //Add the mesh to the drawcommand

    // unsigned int existingVertexCount = vertexOffset / sizeof(Vertex);

    // GPUMemoryHandle cmd = GPUMemoryHandle();
    // cmd.count = mesh.indices.size();
    // cmd.instanceCount = 1;
    // cmd.indexOffset = indexOffset / sizeof(unsigned int);
    // cmd.vboOffset = existingVertexCount;
    // cmd.baseInstance = 0;
    // cmd.ssboIndex = transformIndex;


    //commands.push_back(cmd);


    
    //Add the data to the ibo and vbo
    vb.AddData(vertexOffset, sizeof(Vertex) * mesh.vertices.size(), (const void*)mesh.vertices.data());
    vertexOffset += mesh.vertices.size() * sizeof(Vertex);

    ib.AddData(indexOffset, sizeof(unsigned int) * mesh.indices.size(), (const void*)mesh.indices.data());
    indexOffset += mesh.indices.size() * sizeof(unsigned int);



    std::cout << "\n--- AddMesh Debug ---\n";

    std::cout << "Mesh Vertices: " << mesh.vertices.size() << "\n";
    std::cout << "Mesh Indices: " << mesh.indices.size() << "\n";

    //std::cout << "existingVertexCount: " << existingVertexCount << "\n";
    
    std::cout << "vertexOffset (bytes): " << vertexOffset << "\n";
    std::cout << "indexOffset (bytes): " << indexOffset << "\n";

    //std::cout << "cmd.count: " << cmd.count << "\n";
    //std::cout << "cmd.indexOffset (index elements): " << cmd.indexOffset << "\n";
    //std::cout << "cmd.baseVertex (vertex elements): " << cmd.vboOffset << "\n";
    //std::cout << "cmd.instanceCount: " << cmd.instanceCount << "\n";

    std::cout << "----------------------\n\n";
}

void BufferedBatch::Remove(int index)
{
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
    // glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    // glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * transforms.size(), transforms.data(), GL_DYNAMIC_COPY);
    // glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

    UpdateCommandBuffer();

}

void BufferedBatch::Draw()
{
    //glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);

    glMultiDrawElementsIndirect(
    GL_TRIANGLES, 
    GL_UNSIGNED_INT, 
    nullptr, 
    drawCommands.size(), 
    sizeof(GPUMemoryHandle)
);



}


// void BufferedBatch::ClearBufferedData()
// {
//     ib.Clear();
//     vertexOffset = 0;
//     indexOffset = 0;
    


//     vb.Bind();
//     glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, nullptr, GL_DYNAMIC_DRAW);
    
//     ib.Bind();
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, nullptr, GL_DYNAMIC_DRAW);

//     glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indBuffer);
//     glBufferData(GL_DRAW_INDIRECT_BUFFER,
//                  commands.capacity() * sizeof(GPUMemoryHandle),
//                  nullptr, 
//                  GL_DYNAMIC_DRAW);

    
//     glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
//     glBufferData(GL_SHADER_STORAGE_BUFFER,
//                  transforms.capacity() * sizeof(glm::mat4),
//                  nullptr, 
//                  GL_DYNAMIC_COPY);

//     commands.clear();
//     transforms.clear();

// }

void BufferedBatch::UpdateCommandBuffer(){
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER,
             drawCommands.size() * sizeof(GPUMemoryHandle),
             drawCommands.data(),
             GL_DYNAMIC_DRAW);
}


/*This should be changed to a better alternative than recreating the whole buffer*/
// void BufferedBatch::ClearAndRebuild(const std::vector<std::unique_ptr<Entity>> &entities)
// {
//     ClearBufferedData();

//     // for(const auto& entity_ptr : entities){
//     //     if (entity_ptr && entity_ptr->hasMesh) {
//     //         const Entity& entity = *entity_ptr;
            
//     //         AddMesh(entity.mesh); 
    
//     //         entity_ptr->batchTransformIndex = AddTransform(entity.transform.GetCombined()); 
//     //     }
//     // }
    
//     UpdateCommandBuffer();

//     Bind();
// }

// void Batch::Append(Entity &e)
// {
//     entities.push_back(e.GetHandle());

//     MeshComponent& meshComp = e.GetComponent<MeshComponent>();
//     AddMesh(meshComp.MeshComponent);
//     meshComp.batchTransformIdx = AddTransform(e.GetComponent<TransformComponent>().GetCombined());
// }
