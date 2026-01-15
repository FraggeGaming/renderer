#include "Renderer.h"
#include "../../Renderer/Batch.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "../../Engine.h"
#include "../../ChunkManager.h"

/*
    TODO:
    Plan for the universal renderer system:
    - Support multiple batches with different shaders and layouts
    - Textures and Material support
    - Implement Lightning System (Mby its own system? probably)
    - Support for different rendering techniques/types:
        : Static meshes
        : Skeletal meshes (Animations)
        : Particles
        : Highly Dynamic meshes
        : Terrain meshes (Chunking and dynamic LOD)
        : Billboards
        : Anti Aliasing
    
    


*/



glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
glm::mat4 view;

glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 3.0f);
float ambient = 0.2;


Renderer::Renderer(Shader shader, VertexBufferLayout layout)
{
    batch = std::make_unique<Batch>(shader, 1024*1024* 1024, 1024*1024, 500*64);
    batch->AddLayout(layout);
}

void Renderer::CreateBatch(Shader shader, VertexBufferLayout layout)
{
    batch = std::make_unique<Batch>(shader);
    batch->AddLayout(layout);
}

void Renderer::SetCamera(glm::mat4 CameraMatrix)
{
    view = CameraMatrix;
}

void Renderer::Add(Entity id, MeshCapsule &m, glm::mat4 t)
{
    //LoadMesh(id, m, t);

    chunkMgr.AddToChunk(engine->ecs->GetComponent<TransformComponent>(id), id);
}


bool Renderer::LoadMesh(Entity id, MeshCapsule &m, glm::mat4 t)
{
    Timer timer("LoadMesh", true);
    Mesh mesh = engine->assetManager.Get(m.meshID);

    GPUMemoryHandle handle = batch->Load(m, mesh, t);

    if (!handle.IsValid()) {
        std::cout << "Failed to load mesh - handle is invalid" << std::endl;
        return false;
    }
    
    //engine->ecs->AddComponent<GPUMemoryHandle>(id, handle);
    m.isLoaded = true;
    return true;
}


void Renderer::UnloadMesh(Entity id)
{
    MeshCapsule& m = ecs->GetComponent<MeshCapsule>(id);
    m.isLoaded = false;

    GPUMemoryHandle& handle = ecs->GetComponent<GPUMemoryHandle>(id);
    batch->Unload(handle);
    //ecs->RemoveComponent<GPUMemoryHandle>(id);

}

void Renderer::Remove(Entity id)
{
    UnloadMesh(id);

    chunkMgr.RemoveFromChunk(ecs->GetComponent<TransformComponent>(id), id);
    
}

void Renderer::SetProjection(glm::mat4 mat)
{
}


void Renderer::Start()
{
    std::cout << "Renderer System Started" << std::endl;
    ecs->view<MeshCapsule, TransformComponent>().each([&](int entityId, MeshCapsule& m, TransformComponent& t) {
        if(m.isLoaded == false){
            Add(entityId, m, t.GetCombined());
        }
    });

    std::cout << "Renderer System Initialized with " << batch->geometryRegistry.size() << " unique meshes." << std::endl;
}

void Renderer::Update(float dt)
{
    projection = glm::perspective(glm::radians(90.0f), (float)engine->width/engine->height, 0.1f, 100.0f);
    engine->camera.CalculateFrustum((float)engine->width / (float)engine->height, 90.0f, 0.1f, 100.0f);

    TransformComponent CameraTransform;
    CameraTransform.position = engine->camera.m_cameraPosition;

    std::unordered_map<int, std::vector<int>> meshGroups;
    std::vector<int> lookupTable;
    std::vector<GPUMemoryHandle> finalCommands;

    if (UseChunking){
        {
            Timer t("FetchChunk", true);
            FetchChunk(CameraTransform, 0, 0, 0);
        }
        
        {
            Timer t("Iterate Chunks", true);
            std::vector<ChunkPos> loadedChunks = chunkMgr.activeChunks;

            //Get all loaded positions
            for(ChunkPos pos : loadedChunks){
                Chunk& chunk = chunkMgr.chunks[pos];
                if(!chunk.isVisible) continue;

                for(EntityDrawInfo e : chunk.gpuHandles){
                    TransformComponent& t = ecs->GetComponent<TransformComponent>(e.entity);

                    //Manual transform update for chunked entities 
                    if (t.isDirty) {
                        chunkMgr.ValidateEntityLocation(t, e.MeshID, e.entity, pos);
                    }

                    //Frustum culling and transform update
                    TryRender(meshGroups, t, e.MeshID, e.handle);
                }
            }
        }
    }
    else {
        Timer t("ECS View Iteration", true);
        ecs->view<MeshCapsule, GPUMemoryHandle, TransformComponent>().each([&](int entityId, MeshCapsule& m, GPUMemoryHandle& h, TransformComponent& t) {
            TryRender(meshGroups, t, m.meshID, h);
        });
    }
    
    {
        Timer t("Create Draw Commands", true);
        int count = 0;
        for (auto& [meshID, instances] : meshGroups) {
            count += instances.size();
            MeshGeometryInfo& geo = batch->geometryRegistry[meshID];
            
            GPUMemoryHandle cmd = {};
            cmd.count = geo.indexCount;
            cmd.instanceCount = instances.size();
            cmd.indexOffset = geo.indexOffset;
            cmd.vboOffset = geo.vboOffset;
            cmd.baseInstance = lookupTable.size();

            finalCommands.push_back(cmd);
            lookupTable.insert(lookupTable.end(), instances.begin(), instances.end());
        }
    }

    {
        Timer t("Update Buffers", true);
        batch->UpdateInstanceLookupBuffer(lookupTable);
        batch->SetDrawVector(finalCommands);
    }

    SetCamera(engine->camera.GetView());
}

void Renderer::TryRender(std::unordered_map<int, std::vector<int>>& meshGroups, TransformComponent& t, int meshID, GPUMemoryHandle& h)
{

    if (t.isDirty) {
        batch->UpdateTransform(h.ssboIndex, t.GetCombined());
        t.isDirty = false;
    }
    
    //Frustum Culling
    if (!engine->camera.isVisible(t)) return;
    
    meshGroups[meshID].push_back(h.ssboIndex);
}


void Renderer::FetchChunk(TransformComponent& t, int xOffset, int yOffset, int zOffset){
    Timer timer("FetchChunk TOTAL", true);
    
    auto LoadChunk = [&](TransformComponent& t, int xOffset, int yOffset, int zOffset, Chunk& r){
        Timer loadTimer("LoadChunk", true);
        
        for(Entity e : r.entities){
            MeshCapsule& m = ecs->GetComponent<MeshCapsule>(e);
            bool entStatus = LoadMesh(e, m, ecs->GetComponent<TransformComponent>(e).GetCombined());
            if(entStatus){
                r.LoadHandle(e, ecs->GetComponent<GPUMemoryHandle>(e), m.meshID);
            } else {
                std::cout << "Failed to load entity, attempting to free space..." << std::endl;
                chunkMgr.UnloadOldestChunk(batch.get(), ecs);
                // Retry after unloading
                entStatus = LoadMesh(e, m, ecs->GetComponent<TransformComponent>(e).GetCombined());
            }

            if(entStatus){
                r.LoadHandle(e, ecs->GetComponent<GPUMemoryHandle>(e), m.meshID);
            } else {
                std::cout << "Failed to load entity after unloading a chunk.." << std::endl;
            }
        }
        chunkMgr.Load(t, xOffset, yOffset, zOffset);
    };

    for (int i = -chunkMgr.loadRadius; i <= chunkMgr.loadRadius; i++){
        
        for(int k = -chunkMgr.loadRadius; k <= chunkMgr.loadRadius; k++){
            Chunk& r = chunkMgr.Get(t, i, 0, k);
            if(!r.isLoaded){
                LoadChunk(t, i, 0, k, r);
            } 
            else if (!r.isVisible){
                r.isVisible = true;
            }
        }
        
    }


    //TODO: Unload distant chunks
    std::vector<ChunkPos> toRemove;
    for (size_t i = 0; i < chunkMgr.activeChunks.size(); i++)
    {
        Chunk& chunk = chunkMgr.chunks[chunkMgr.activeChunks[i]];

        int diffX = std::abs(chunkMgr.activeChunks[i].x - (int)glm::floor(t.position.x / chunkMgr.chunkSize));
        int diffY = std::abs(chunkMgr.activeChunks[i].y - (int)glm::floor(t.position.y / chunkMgr.chunkSize));
        int diffZ = std::abs(chunkMgr.activeChunks[i].z - (int)glm::floor(t.position.z / chunkMgr.chunkSize));  
        if (std::max({diffX, diffY, diffZ}) > chunkMgr.loadRadius) {
            toRemove.push_back(chunkMgr.activeChunks[i]);
            
        }
    }

    for(ChunkPos p : toRemove){
        chunkMgr.Unload(p).UnLoad(batch.get(), ecs);
    }


}

void Renderer::Render()
{
    Timer timer("Renderer::Render", true);
    
    batch->shader.Use();
    batch->shader.SetMat4("view", glm::value_ptr(view), GL_FALSE);
    batch->shader.SetMat4("projection", glm::value_ptr(projection), GL_FALSE);

    batch->shader.SetVec3("lightPos", glm::value_ptr(lightPosition));
    batch->shader.SetFloat("ambient", ambient);

    {
        Timer t("Batch Bind", true);
        batch->Bind();
    }
    
    {
        Timer t("Batch Draw", true);
        batch->Draw();
    }
}

void Renderer::DebugTrace()
{
    // Profiling
    ProfileData::PrintStats();
    ProfileData::Reset();
    
    std::cout << "\n=== Renderer Debug Trace ===" << std::endl;
    std::cout << "\n--- Vertex Buffer ---" << std::endl;
    batch->vb.GetDebugInfo();
    
    std::cout << "\n--- Index Buffer ---" << std::endl;
    batch->ib.GetDebugInfo();
    
    std::cout << "\n--- SSBO Buffer (Transforms) ---" << std::endl;
    batch->transformBuffer.GetDebugInfo(false); // Dont print the buffer contents
    
    std::cout << "\n--- Geometry Registry ---" << std::endl;
    std::cout << "Total registered geometries: " << batch->geometryRegistry.size() << std::endl; 
    std::cout << "\n--- Draw Commands ---" << std::endl;
    std::cout << "Total draw commands: " << batch->drawCommands.size() << std::endl;
    
    std::cout << "=== End Debug Trace ===\n" << std::endl;
}
