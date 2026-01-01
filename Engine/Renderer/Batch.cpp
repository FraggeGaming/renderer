#include "Batch.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../Components/MeshComponent.h"
#include "../Components/TransformComponent.h"

//Fix this, let it have the ECS and let it just go through the 


Batch::Batch(Shader shader)
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

Batch::~Batch()
{
}

int Batch::AddTransform(glm::mat4 t)  
{
    int size = transforms.size();
    transforms.push_back(t);

    return size;
}

void Batch::AddMesh(Mesh mesh)
{

    unsigned int instanceIndex = commands.size();
    //Add the mesh to the drawcommand
    unsigned int existingVertexCount = vertexOffset / sizeof(Vertex);
    //unsigned int firstIndex = indexOffset / sizeof(unsigned int);

    DrawElementsIndirectCommand cmd = DrawElementsIndirectCommand();
    cmd.count = mesh.indices.size();
    cmd.instanceCount = 1;
    cmd.firstIndex = indexOffset / sizeof(unsigned int);
    cmd.baseVertex = existingVertexCount;
    cmd.baseInstance = 0;

    commands.push_back(cmd);

    
    //Add the data to the ibo and vbo

    vb.AddData(vertexOffset, sizeof(Vertex) * mesh.vertices.size(), (const void*)mesh.vertices.data());
    vertexOffset += mesh.vertices.size() * sizeof(Vertex);

    // std::vector<unsigned int> adj;
    // adj.reserve(mesh.indices.size());
    // for (auto idx : mesh.indices)
    //     adj.push_back(idx + existingVertexCount);


    // std::cout << adj.size();
    
        
    //ib.AddData(indexOffset, sizeof(unsigned int) * adj.size(), (const void*)adj.data());
    ib.AddData(indexOffset, sizeof(unsigned int) * mesh.indices.size(), (const void*)mesh.indices.data());
    indexOffset += mesh.indices.size() * sizeof(unsigned int);



    std::cout << "\n--- AddMesh Debug ---\n";

    std::cout << "Mesh Vertices: " << mesh.vertices.size() << "\n";
    std::cout << "Mesh Indices: " << mesh.indices.size() << "\n";

    std::cout << "existingVertexCount: " << existingVertexCount << "\n";
    
    std::cout << "vertexOffset (bytes): " << vertexOffset << "\n";
    std::cout << "indexOffset (bytes): " << indexOffset << "\n";

    std::cout << "cmd.count: " << cmd.count << "\n";
    std::cout << "cmd.firstIndex (index elements): " << cmd.firstIndex << "\n";
    std::cout << "cmd.baseVertex (vertex elements): " << cmd.baseVertex << "\n";
    std::cout << "cmd.instanceCount: " << cmd.instanceCount << "\n";

    std::cout << "----------------------\n\n";
}

void Batch::RemoveMesh(int index)
{
    commands.erase(commands.begin() + index);
    transforms.erase(transforms.begin() + index);

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

    //bind ssbo for transforms
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4) * transforms.size(), transforms.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

}

void Batch::Draw()

{
    

    //glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);

    glMultiDrawElementsIndirect(
    GL_TRIANGLES, 
    GL_UNSIGNED_INT, 
    nullptr, 
    commands.size(), 
    sizeof(DrawElementsIndirectCommand)
);



}


void Batch::ClearBufferedData()
{
    ib.Clear();
    vertexOffset = 0;
    indexOffset = 0;
    


    vb.Bind();
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, nullptr, GL_DYNAMIC_DRAW);
    
    ib.Bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, nullptr, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER,
                 commands.capacity() * sizeof(DrawElementsIndirectCommand),
                 nullptr, 
                 GL_DYNAMIC_DRAW);

    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 transforms.capacity() * sizeof(glm::mat4),
                 nullptr, 
                 GL_DYNAMIC_COPY);

    commands.clear();
    transforms.clear();

}

void Batch::UpdateCommandBuffer(){
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER,
             commands.size() * sizeof(DrawElementsIndirectCommand),
             commands.data(),
             GL_DYNAMIC_DRAW);

        std::cout << "Updated indirect buffer";
}


/*This should be changed to a better alternative than recreating the whole buffer*/
// void Batch::ClearAndRebuild(const std::vector<std::unique_ptr<Entity>> &entities)
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
