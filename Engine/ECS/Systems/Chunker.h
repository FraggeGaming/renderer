#pragma once
#include "System.h"
#include "../../Components/MeshCapsule.h"
#include "../../Renderer/Batch.h"
#include "../../Engine.h"
#include "Renderer.h"
#include "../../ChunkManager.h"

class Chunker : public System{

private:
    std::vector<GPUData> data;

    void SetLoaded(Chunk& chunk, ChunkPos& pos);

    void SetUnloaded(Chunk& chunk , ChunkPos& pos);

    ChunkPos Get(TransformComponent& t, float divider);

    ChunkPos Get(TransformComponent& t, int xoffset, int yoffset, int zoffset, float divider);

    bool LoadChunk(ChunkPos p, Batch* batch);

    void UnloadChunk(ChunkPos p, Batch* batch);

    void LoadBounded(Batch* batch);
    void UnloadUnbounded(Batch* batch);

    void ThreadedChunker(Batch* batch);

public:
    std::unordered_map<ChunkPos, Chunk> chunks;
    ChunkPos loadReference = {0, 0, 0};
    float chunkSize = 10.0f;
    int loadRadius = 3;

    std::list<ChunkPos> loaded;

    void Start() override;
    void Update(float dt) override;
    void Render() override;

    void Add(TransformComponent& t, Entity e);
    void Remove(TransformComponent& t, Entity e);

    
    bool InLoadedBounds(ChunkPos& p);

    void SetLocation(TransformComponent& t);

    
    void ValidateEntityLocation(TransformComponent& t, int meshID, Entity e, ChunkPos oldPos);

    std::vector<EntityDrawInfo> GenerateCommands();
};
