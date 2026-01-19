#include "Renderer.h"
#include "../../Renderer/Batch.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>

#include "../../Engine.h"
#include "../../ChunkManager.h"
#include "Chunker.h"

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


GPUMemoryHandle Renderer::LoadMesh(Entity id, MeshCapsule &m, glm::mat4 t)
{
    Timer timer("LoadMesh", true);
    Mesh mesh = engine->assetManager.Get(m.meshID);

    GPUMemoryHandle handle = batch->Load(m, mesh, t);

    if (handle.IsValid()) {
        m.isLoaded = true;
    }
    
    return handle;
}


void Renderer::UnloadMesh(Entity id)
{
    MeshCapsule& m = ecs->GetComponent<MeshCapsule>(id);
    m.isLoaded = false;

    GPUMemoryHandle& handle = ecs->GetComponent<GPUMemoryHandle>(id);
    batch->Unload(handle);
    //ecs->RemoveComponent<GPUMemoryHandle>(id);

}


void Renderer::SetProjection(glm::mat4 mat)
{
}


void Renderer::Start()
{
    std::cout << "Renderer System Started" << std::endl;
    Chunker* objectChunker = engine->GetSystem<Chunker>();

    ecs->view<MeshCapsule, TransformComponent>().each([&](int entityId, MeshCapsule& m, TransformComponent& t) {
    
        if(m.mode == MeshMode::CHUNKED){
            objectChunker->Add(t, entityId);
        }

        else if(m.mode == MeshMode::STATIC){
            //objectChunker->Add(t, entityId);
        }

        else if(m.mode == MeshMode::DYNAMIC){
            //objectChunker->Add(t, entityId);
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
    engine->GetSystem<Chunker>()->SetLocation(CameraTransform);

    //Track the mesh instances
    std::unordered_map<int, std::vector<int>> meshGroups;

    //Track the transform index for the ssbo
    std::vector<int> lookupTable;

    //Final draw commands to be sent to the GPU
    std::vector<GPUMemoryHandle> finalCommands;

    std::vector<EntityDrawInfo> infoCommands = engine->GetSystem<Chunker>()->GenerateCommands();
    for(EntityDrawInfo e : infoCommands){
        TransformComponent& t = ecs->GetComponent<TransformComponent>(e.entity);
        
        //Validate that entity is in right chunk
        //If not, move it to the correct one
        //Chunker.validateLoaction

        //Frustum culling and transform update
        TryRender(meshGroups, t, e.MeshID, e.handle);
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
