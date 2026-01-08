#include "BatchedRenderer.h"
#include "../../Renderer/BufferedBatch.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>


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

void BatchedRenderer::Add(Entity id, BufferedMesh &m, glm::mat4 t)
{
    LoadMesh(id, m, t);

    chunkMgr.AddToChunk(engine->ecs->GetComponent<TransformComponent>(id), id);
}

//Change to private
bool BatchedRenderer::LoadMesh(Entity id, BufferedMesh &m, glm::mat4 t)
{
    Mesh mesh = engine->assetManager.Get(m.meshID);
    GPUMemoryHandle handle = batch->Load(m, mesh, t);

    if (handle.ssboIndex == -1) return false; // or check appropriate error condition
    
    engine->ecs->AddComponent<GPUMemoryHandle>(id, handle);
    m.isLoaded = true;
    return true;
}

//Change to private
void BatchedRenderer::UnloadMesh(Entity id)
{
    BufferedMesh& m = ecs->GetComponent<BufferedMesh>(id);
    m.isLoaded = false;

    GPUMemoryHandle& handle = ecs->GetComponent<GPUMemoryHandle>(id);
    batch->Unload(handle);
    ecs->RemoveComponent<GPUMemoryHandle>(id);

}

void BatchedRenderer::Remove(Entity id)
{
    UnloadMesh(id);

    chunkMgr.RemoveFromChunk(ecs->GetComponent<TransformComponent>(id), id);
    
}

void BatchedRenderer::SetProjection(glm::mat4 mat)
{
}


void BatchedRenderer::Start()
{
    ecs->view<BufferedMesh, TransformComponent>().each([&](int entityId, BufferedMesh& m, TransformComponent& t) {
        if(m.isLoaded == false){
            Add(entityId, m, t.GetCombined());
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

    
    //Regular frustum culling and batching
    std::unordered_map<int, std::vector<int>> meshGroups;
    std::vector<int> lookupTable;
    std::vector<GPUMemoryHandle> finalCommands;


    if (UseChunking){
        FetchChunk(CameraTransform, 0, 0, 0);
        std::vector<ChunkPos> loadedChunks = chunkMgr.activeChunks;
        for(ChunkPos pos : loadedChunks){
            Chunk& chunk = chunkMgr.chunks[pos];
            for(Entity e : chunk.entities){
                if(!ecs->HasComponent<GPUMemoryHandle>(e)) continue;
                GPUMemoryHandle& h = ecs->GetComponent<GPUMemoryHandle>(e);;
                TransformComponent& t = ecs->GetComponent<TransformComponent>(e);

                if (t.isDirty) {
                    batch->UpdateTransform(h.ssboIndex, t.GetCombined());
                    chunkMgr.ValidateEntityLocation(t, e, pos);
                    t.isDirty = false;
                }

                if (!engine->camera.isVisible(t)) continue;

                if(!ecs->HasComponent<BufferedMesh>(e)) continue;
                BufferedMesh& m = ecs->GetComponent<BufferedMesh>(e);
                    
                meshGroups[m.meshID].push_back(h.ssboIndex);
            }
        }
    }

    else {
        ecs->view<BufferedMesh, GPUMemoryHandle, TransformComponent>().each([&](int entityId, BufferedMesh& m, GPUMemoryHandle& h, TransformComponent& t) {
            
            if (t.isDirty) {
                batch->UpdateTransform(h.ssboIndex, t.GetCombined());
                t.isDirty = false;
            }
            
            //Frustum Culling
            if (!engine->camera.isVisible(t)) return;
            
            meshGroups[m.meshID].push_back(h.ssboIndex);
        });
    }
    

 
    //Create draw commands
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

void BatchedRenderer::FetchChunk(TransformComponent& t, int xOffset, int yOffset, int zOffset){

    auto LoadChunk = [&](TransformComponent& t, int xOffset, int yOffset, int zOffset, Chunk& r){
        std::cout << "Loading Chunk at (" << (int)glm::floor(t.position.x / chunkMgr.chunkSize) + xOffset << ", "
                  << (int)glm::floor(t.position.y / chunkMgr.chunkSize) + yOffset << ", "
                  << (int)glm::floor(t.position.z / chunkMgr.chunkSize) + zOffset << ")" << std::endl;
        
        //Load meshes for all entities in chunk
        for(Entity e : r.entities){
            BufferedMesh& m = ecs->GetComponent<BufferedMesh>(e);
            bool entStatus = LoadMesh(e, m, ecs->GetComponent<TransformComponent>(e).GetCombined());
            if(entStatus){
                r.LoadHandle(e, ecs->GetComponent<GPUMemoryHandle>(e));
            }

            //If entstatus = false, unload oldest chunk and try again
            
        }
        chunkMgr.Load(t, xOffset, yOffset, zOffset);
    };


    Chunk r = chunkMgr.Get(t, 0, 0, 0);
    
    if(!r.isLoaded){
        LoadChunk(t, 0, 0, 0, r);
    }

    Chunk r1 = chunkMgr.Get(t, 1, 0, 0);
    
    if(!r1.isLoaded){
        LoadChunk(t, 1, 0, 0, r1);
    }

    //r.UnLoad(*batch, *ecs);
    
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

void BatchedRenderer::DebugTrace()
{
    std::cout << "\n=== BatchedRenderer Debug Trace ===" << std::endl;
    std::cout << "\n--- Vertex Buffer ---" << std::endl;
    batch->vb.GetDebugInfo();
    
    std::cout << "\n--- Index Buffer ---" << std::endl;
    batch->ib.GetDebugInfo();
    
    //std::cout << "\n--- SSBO Buffer (Transforms) ---" << std::endl;
    //batch->ssboBuffer.GetDebugInfo();
    
    std::cout << "\n--- Geometry Registry ---" << std::endl;
    std::cout << "Total registered geometries: " << batch->geometryRegistry.size() << std::endl; 
    std::cout << "\n--- Draw Commands ---" << std::endl;
    std::cout << "Total draw commands: " << batch->drawCommands.size() << std::endl;
    
    std::cout << "=== End Debug Trace ===\n" << std::endl;
}
