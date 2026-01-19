#include "Chunker.h"

void Chunker::SetLoaded(Chunk& chunk, ChunkPos& pos){
    chunk.isLoaded = true;
    loaded.push_back(pos);
}

void Chunker::SetUnloaded(Chunk& chunk , ChunkPos& pos){
    std::cout << "[Chunker] SetUnloaded called for " << pos.toString() << std::endl;
    chunk.isLoaded = false;
    std::cout << "[Chunker] Chunk marked as unloaded" << std::endl;
}

void Chunker::Add(TransformComponent& t, Entity e){
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize),
            (int)glm::floor(t.position.y / chunkSize),
            (int)glm::floor(t.position.z / chunkSize)
        };
        chunks[pos].Add(e);
    }

    void Chunker::Remove(TransformComponent& t, Entity e){
        ChunkPos pos = {
            (int)glm::floor(t.position.x / chunkSize),
            (int)glm::floor(t.position.y / chunkSize),
            (int)glm::floor(t.position.z / chunkSize)
        };
        chunks[pos].Remove(e);
        
    }

ChunkPos Chunker::Get(TransformComponent& t, float divider){
    return {
        (int)glm::floor(t.position.x / divider),
        (int)glm::floor(t.position.y / divider),
        (int)glm::floor(t.position.z / divider)
    };
    
}

ChunkPos Chunker::Get(TransformComponent& t, int xoffset, int yoffset, int zoffset, float divider){
    return {
        (int)glm::floor(t.position.x / divider) + xoffset,
        (int)glm::floor(t.position.y / divider) + yoffset,
        (int)glm::floor(t.position.z / divider) + zoffset
    };
    
}

bool Chunker::LoadChunk(ChunkPos p, Batch* batch){
    Chunk& chunk = chunks[p];
    if(chunk.isLoaded) return true;


    for(size_t i = 0; i < chunk.entities.size(); i++){
        Entity entity = chunk.entities[i];
        int meshID = ecs->GetComponent<MeshCapsule>(entity).meshID;
        Mesh& mesh = engine->assetManager.Get(meshID);
        TransformComponent& transform = ecs->GetComponent<TransformComponent>(entity);
        GPUMemoryHandle handle = batch->Load(meshID, mesh, transform.GetCombined());
        if (!handle.IsValid()) {
            std::cout << "[Chunker] ERROR: Failed to load mesh " << meshID << " for entity " << entity << " - handle is invalid" << std::endl;
            return false;
        } else {
            chunk.LoadHandle(entity, handle, meshID);
        }
    }
    SetLoaded(chunk, p);
    return true;
}

void Chunker::UnloadChunk(ChunkPos p, Batch* batch){
    Chunk& chunk = chunks[p];
    if(!chunk.isLoaded) return;


    for(size_t i = 0; i < chunk.entities.size(); i++){
        Entity entity = chunk.entities[i];

        EntityDrawInfo drawinfo = chunk.gpuHandles[chunk.entityToHandleIndex[entity]];
        GPUMemoryHandle handle = drawinfo.handle;
        batch->Unload(handle);

    }
    chunk.isLoaded = false;

}

void Chunker::Start() {
    Timer t("Chunker start", true);
    chunkSize = 32.0f;
    loadRadius = 2;

    std::cout << "[Chunker] Start - Adding entities to chunks" << std::endl;
    ecs->view<MeshCapsule, TransformComponent>().each([&](int entityId, MeshCapsule& m, TransformComponent& t) {
        Add(t, entityId);
    });
    std::cout << "[Chunker] Start complete - Total chunks with entities: " << chunks.size() << std::endl;



}

void Chunker::Update(float dt) {

    Batch* batch = engine->GetSystem<Renderer>()->batch.get();

    //Fetch and load chunks in the loadradius
    for (int i = -loadRadius; i <= loadRadius; i++){
        for(int k = -loadRadius; k <= loadRadius; k++){
            ChunkPos reference = {loadReference.x + i, loadReference.y, loadReference.z + k};
            LoadChunk(reference, batch);
        }
    }

    //Unload chunks outside the radius
    std::list<ChunkPos>::iterator it = loaded.begin();
    while (it != loaded.end()) {
        if (!InLoadedBounds(*it)) {
            UnloadChunk(*it, batch);
            it = loaded.erase(it);
            continue;
        }
        it++;
    }
}

void Chunker::Render() {

}

bool Chunker::InLoadedBounds(ChunkPos& p){
    
    if(loadReference.x + loadRadius < p.x || loadReference.x - loadRadius > p.x
    || loadReference.y + loadRadius < p.y || loadReference.y - loadRadius > p.y
    || loadReference.z + loadRadius < p.z || loadReference.z - loadRadius > p.z
    ){
        return false;
    }

    return true;
}

void Chunker::SetLocation(TransformComponent& t){
    ChunkPos newRef = Get(t, chunkSize);

    
    if(loadReference != newRef){
        std::cout << loadReference.toString() << ", NEW: " << newRef.toString() << std::endl;
        loadReference.Set(t, chunkSize);        
    }
}

std::vector<EntityDrawInfo> Chunker::GenerateCommands(){
    std::vector<EntityDrawInfo> finalCommands;


    std::list<ChunkPos>::iterator it;
    for (it = loaded.begin(); it != loaded.end(); ++it)
    {
        Chunk& ref = chunks[*it];
        finalCommands.insert(finalCommands.end(),ref.gpuHandles.begin(), ref.gpuHandles.end());
    }
    
    return finalCommands;

}

void Chunker::ValidateEntityLocation(TransformComponent& t, int meshID, Entity e, ChunkPos oldPos){
    // ChunkPos currentPos = {
    //     (int)glm::floor(t.position.x / chunkSize),
    //     (int)glm::floor(t.position.y / chunkSize),
    //     (int)glm::floor(t.position.z / chunkSize)
    // };

    // if(currentPos == oldPos) return;

    // std::cout << "ChunkManager: Moving entity " << e << " from chunk (" 
    //             << oldPos.x << ", " << oldPos.y << ", " << oldPos.z << ") to chunk ("
    //             << currentPos.x << ", " << currentPos.y << ", " << currentPos.z << ")" << std::endl;
    // GPUMemoryHandle handle = chunks[oldPos].Remove(e);

    // chunks[currentPos].Add(e);
    // chunks[currentPos].LoadHandle(e, handle, meshID);
}