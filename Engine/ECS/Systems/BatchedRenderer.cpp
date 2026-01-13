#include "BatchedRenderer.h"
#include "../../Renderer/BufferedBatch.h"

#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <iomanip>



#include "../../Engine.h"

#include "../../ChunkManager.h"

// Profiling system
class ProfileData {
public:
    struct Entry {
        std::string name;
        long long totalMicroseconds = 0;
        int callCount = 0;
        
        double getAverageMicros() const { return callCount > 0 ? (double)totalMicroseconds / callCount : 0.0; }
        double getAverageMillis() const { return getAverageMicros() / 1000.0; }
    };
    
private:
    static inline std::unordered_map<std::string, Entry> entries;
    
public:
    static void Record(const std::string& name, long long microseconds) {
        entries[name].name = name;
        entries[name].totalMicroseconds += microseconds;
        entries[name].callCount++;
    }
    
    static void PrintStats() {
        std::cout << "\n=== PROFILING STATISTICS ===" << std::endl;
        std::cout << std::left << std::setw(35) << "Function" 
                  << std::right << std::setw(12) << "Calls" 
                  << std::setw(15) << "Total (ms)" 
                  << std::setw(15) << "Avg (µs)" 
                  << std::setw(15) << "Avg (ms)" << std::endl;
        std::cout << std::string(92, '-') << std::endl;
        
        // Sort by total time
        std::vector<Entry> sorted;
        for (const auto& [name, entry] : entries) {
            sorted.push_back(entry);
        }
        std::sort(sorted.begin(), sorted.end(), [](const Entry& a, const Entry& b) {
            return a.totalMicroseconds > b.totalMicroseconds;
        });
        
        for (const auto& entry : sorted) {
            std::cout << std::left << std::setw(35) << entry.name
                      << std::right << std::setw(12) << entry.callCount
                      << std::setw(15) << std::fixed << std::setprecision(3) << (entry.totalMicroseconds / 1000.0)
                      << std::setw(15) << std::fixed << std::setprecision(2) << entry.getAverageMicros()
                      << std::setw(15) << std::fixed << std::setprecision(3) << entry.getAverageMillis()
                      << std::endl;
        }
        std::cout << "========================\n" << std::endl;
    }
    
    static void Reset() {
        entries.clear();
    }
};

class Timer {
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
    std::string name;
    bool silent;
public:
    Timer(const std::string& name, bool silent = false) 
        : name(name), silent(silent), start(std::chrono::high_resolution_clock::now()) {}
    
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        ProfileData::Record(name, duration);
        
        if(!silent && duration > 50) {
            std::cout << name << ": " << duration << " µs (" << duration/1000.0 << " ms)" << std::endl;
        }
    }
};

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
    Timer timer("LoadMesh", true);
    Mesh mesh = engine->assetManager.Get(m.meshID);
    GPUMemoryHandle handle = batch->Load(m, mesh, t);

    if (handle.ssboIndex == -1) return false;
    
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
    Timer timer("BatchedRenderer::Start", true);
    ecs->view<BufferedMesh, TransformComponent>().each([&](int entityId, BufferedMesh& m, TransformComponent& t) {
        if(m.isLoaded == false){
            Add(entityId, m, t.GetCombined());
        }
    });
}

void BatchedRenderer::Update(float dt)
{
    Timer timer("BatchedRenderer::Update TOTAL", true);
    
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

            for(ChunkPos pos : loadedChunks){
                Chunk& chunk = chunkMgr.chunks[pos];
                if(!chunk.isVisible) continue;

                for(EntityDrawInfo e : chunk.gpuHandles){
                    TransformComponent& t = ecs->GetComponent<TransformComponent>(e.entity);

                    if (t.isDirty) {
                        chunkMgr.ValidateEntityLocation(t, e.MeshID, e.entity, pos);
                    }
                    TryRender(meshGroups, t, e.MeshID, e.handle);
                }
            }
        }
    }
    else {
        Timer t("ECS View Iteration", true);
        ecs->view<BufferedMesh, GPUMemoryHandle, TransformComponent>().each([&](int entityId, BufferedMesh& m, GPUMemoryHandle& h, TransformComponent& t) {
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

void BatchedRenderer::TryRender(std::unordered_map<int, std::vector<int>>& meshGroups, TransformComponent& t, int meshID, GPUMemoryHandle& h)
{

    if (t.isDirty) {
        batch->UpdateTransform(h.ssboIndex, t.GetCombined());
        t.isDirty = false;
    }
    
    //Frustum Culling
    if (!engine->camera.isVisible(t)) return;
    
    meshGroups[meshID].push_back(h.ssboIndex);
}


void BatchedRenderer::FetchChunk(TransformComponent& t, int xOffset, int yOffset, int zOffset){
    Timer timer("FetchChunk TOTAL", true);
    
    auto LoadChunk = [&](TransformComponent& t, int xOffset, int yOffset, int zOffset, Chunk& r){
        Timer loadTimer("LoadChunk", true);
        
        for(Entity e : r.entities){
            BufferedMesh& m = ecs->GetComponent<BufferedMesh>(e);
            bool entStatus = LoadMesh(e, m, ecs->GetComponent<TransformComponent>(e).GetCombined());
            if(entStatus){
                r.LoadHandle(e, ecs->GetComponent<GPUMemoryHandle>(e), m.meshID);
            }
        }
        chunkMgr.Load(t, xOffset, yOffset, zOffset);
    };

    for (int i = -chunkMgr.loadRadius; i <= chunkMgr.loadRadius; i++){
        for(int j = -chunkMgr.loadRadius; j <= 1; j++){
            for(int k = -chunkMgr.loadRadius; k <= chunkMgr.loadRadius; k++){
                Chunk& r = chunkMgr.Get(t, i, j, k);
                if(!r.isLoaded){
                    LoadChunk(t, i, j, k, r);
                } 
                else if (!r.isVisible){
                    r.isVisible = true;
                }
            }
        }
    }

    for (size_t i = 0; i < chunkMgr.activeChunks.size(); i++)
    {
        Chunk& chunk = chunkMgr.chunks[chunkMgr.activeChunks[i]];
        if(!chunk.isVisible) continue;

        int diffX = std::abs(chunkMgr.activeChunks[i].x - (int)glm::floor(t.position.x / chunkMgr.chunkSize));
        int diffY = std::abs(chunkMgr.activeChunks[i].y - (int)glm::floor(t.position.y / chunkMgr.chunkSize));
        int diffZ = std::abs(chunkMgr.activeChunks[i].z - (int)glm::floor(t.position.z / chunkMgr.chunkSize));  
        if (std::max({diffX, diffY, diffZ}) > chunkMgr.loadRadius) {
            chunk.isVisible = false;
        }
    }
}

void BatchedRenderer::Render()
{
    Timer timer("BatchedRenderer::Render", true);
    
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

void BatchedRenderer::DebugTrace()
{
    // Profiling
    ProfileData::PrintStats();
    ProfileData::Reset();
    
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
