#include "BatchedRenderer.h"
#include "../../Renderer/BufferedBatch.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "../../Components/BufferedMesh.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/GPUMemoryHandle.h"

#include "../../Engine.h"

#include "../../ChunkManager.h"

ChunkManager chunkMgr;


glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
glm::mat4 view;

glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 3.0f);
float ambient = 0.2;

BatchedRenderer::BatchedRenderer(Shader shader, VertexBufferLayout layout)
{
    batch = std::make_unique<BufferedBatch>(shader);
    batch->AddLayout(layout);
}

void BatchedRenderer::CreateBatch(Shader shader, VertexBufferLayout layout)
{
    batch = std::make_unique<BufferedBatch>(shader);
    batch->AddLayout(layout);
}

void BatchedRenderer::SetCamera(glm::mat4 CameraMatrix)
{
    view = CameraMatrix;
}

void BatchedRenderer::LoadMesh(Entity id, BufferedMesh &m, glm::mat4 t)
{
    Mesh mesh = engine->assetManager.Get(m.meshID);
    GPUMemoryHandle handle = batch->Load(m, mesh, t);

    engine->ecs->AddComponent<GPUMemoryHandle>(id, handle);
    m.isLoaded = true;

    chunkMgr.AddToChunk(engine->ecs->GetComponent<TransformComponent>(id), id);
}

void BatchedRenderer::Unload(Entity id)
{
    BufferedMesh& e = ecs->GetComponent<BufferedMesh>(id);
    e.isLoaded = false;

    GPUMemoryHandle& handle = ecs->GetComponent<GPUMemoryHandle>(id);
    batch->Unload(handle);
    ecs->RemoveComponent<GPUMemoryHandle>(id);

    chunkMgr.RemoveFromChunk(ecs->GetComponent<TransformComponent>(id), id);
    
}

void BatchedRenderer::SetProjection(glm::mat4 mat)
{
}


void BatchedRenderer::Start()
{
    ecs->view<BufferedMesh, TransformComponent>().each([&](int entityId, BufferedMesh& m, TransformComponent& t) {
        if(m.isLoaded == false){
            LoadMesh(entityId, m, t.GetCombined());
        }
    });
}

void BatchedRenderer::Update(float dt)
{
    projection = glm::perspective(glm::radians(90.0f), (float)engine->width/engine->height, 0.1f, 100.0f);
    engine->camera.CalculateFrustum((float)engine->width / (float)engine->height, 90.0f, 0.1f, 100.0f);

    //New chunk testing
    TransformComponent CameraTransform;
    CameraTransform.position = engine->camera.m_cameraPosition;

    chunkMgr.Load(CameraTransform); //Load the chunk where the camera is located

    /*
    Move to better verision later where if loading new chunk, 
    generate GPuMemoryHandles for entities in that chunk

    When unloading chunk, remove GPUMemoryHandles for entities in that chunk

    then use regular render loop

    */

    //Regular frustum culling and batching
    std::unordered_map<int, std::vector<int>> meshGroups;
    std::vector<int> lookupTable;
    std::vector<GPUMemoryHandle> finalCommands;


    std::vector<Entity> visibleEntities = chunkMgr.GetLoadedEntities();
    //std::cout << "Visible Entities in Loaded Chunks: " << visibleEntities.size() << std::endl;
    for(Entity e : visibleEntities){
    
        if(!ecs->HasComponent<GPUMemoryHandle>(e)) continue;
        GPUMemoryHandle& h = ecs->GetComponent<GPUMemoryHandle>(e);;
        TransformComponent& t = ecs->GetComponent<TransformComponent>(e);

        if (t.isDirty) {
            batch->UpdateTransform(h.ssboIndex, t.GetCombined());
            t.isDirty = false;
        }

        if (!engine->camera.isVisible(t)) continue;

        if(!ecs->HasComponent<BufferedMesh>(e)) continue;
        BufferedMesh& m = ecs->GetComponent<BufferedMesh>(e);
            
        meshGroups[m.meshID].push_back(h.ssboIndex);

        //std::cout << "Entity " << e << " with MeshID " << m.meshID << " is visible." << std::endl;
    }

    /*

    //TODO: fetch from chunkmanager
     ecs->view<BufferedMesh, GPUMemoryHandle, TransformComponent>().each([&](int entityId, BufferedMesh& m, GPUMemoryHandle& h, TransformComponent& t) {
        
         if (t.isDirty) {
             batch->UpdateTransform(h.ssboIndex, t.GetCombined());
             t.isDirty = false;
         }
        
         //Frustum Culling
         if (!engine->camera.isVisible(t)) return;
        
         meshGroups[m.meshID].push_back(h.ssboIndex);
     });
     */

    int count = 0;
    for (auto& [meshID, instances] : meshGroups) {
        count += instances.size();
        MeshGeometryInfo& geo = batch->geometryRegistry[meshID];
        
        GPUMemoryHandle cmd = {};
        cmd.count = geo.indexCount;
        cmd.instanceCount = instances.size();
        cmd.indexOffset = geo.indexOffset;
        cmd.vboOffset = geo.vboOffset;
        cmd.baseInstance = lookupTable.size(); // Offset in the lookup SSBO

        finalCommands.push_back(cmd);
        lookupTable.insert(lookupTable.end(), instances.begin(), instances.end());
    }

    //std::cout << "BatchedRenderer::Update - instances this frame: " << count << std::endl;

    batch->UpdateInstanceLookupBuffer(lookupTable);
    batch->SetDrawVector(finalCommands);
    


    SetCamera(engine->camera.GetView());
}

void BatchedRenderer::Render()
{

    batch->shader.Use();
    batch->shader.SetMat4("view", glm::value_ptr(view), GL_FALSE); //Set the view matrix
    batch->shader.SetMat4("projection", glm::value_ptr(projection), GL_FALSE); //Set projection matrix

    //Change this later
    //bind lightning
    batch->shader.SetVec3("lightPos", glm::value_ptr(lightPosition));
    batch->shader.SetFloat("ambient", ambient);


    batch->Bind();
    batch->Draw();
}
