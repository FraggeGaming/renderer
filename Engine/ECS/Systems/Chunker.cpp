// #include "System.h"
// #include "../../ChunkManager.h"
// #include "../../Components/MeshCapsule.h"
// #include "../../Renderer/Batch.h"
// #include "../../Engine.h"
// /*
// We need to render for a couple things

// Static meshes
// Dynamic, mby dont use chunking?

// */


// class Chunker : public System{


//     std::unordered_map<ChunkPos, Chunk> chunks;
//     float chunkSize = 16.0f;
//     int loadRadius = 4;

//     void Start() override {

//     };
//     void Update(float dt) override {

//     };
//     void Render() override {

//     };

//     void SetLocation(TransformComponent& t){

//     };

//     std::vector<GPUMemoryHandle> GenerateCommands(){
//         std::vector<GPUMemoryHandle> finalCommands;
//     };


//     // bool LoadChunk(ChunkPos p, Batch* batch){
//     //     Chunk& chunk = chunks[p];

//     //     if(chunk.isLoaded) return true;

//     //     //Generate GPU handle for all entities and load into batch
//     //     for(size_t entity = 0; entity < chunk.entities.size(); entity++){

//     //         int meshID = ecs->GetComponent<MeshCapsule>(entity).meshID;
//     //         Mesh& mesh = engine->assetManager.Get(meshID);
//     //         TransformComponent& transform = ecs->GetComponent<TransformComponent>(entity);

//     //         //Load
//     //         GPUMemoryHandle handle = batch->Load(meshID, mesh, transform.GetCombined());

//     //         //If it could not be loaded
//     //         if (!handle.IsValid()) {
//     //             std::cout << "Failed to load mesh - handle is invalid" << std::endl;
//     //             return false;
//     //         } else {
//     //             chunk.LoadHandle(entity, handle, meshID);
//     //         }
//     //     }

//     //     //Set Chunk to active
//     //     chunk.isLoaded = true;

//     //     return true;
//     // };

//     // void UnloadChunk(ChunkPos p){
//     //     Chunk& chunk = chunks[p];

//     //     if(!chunk.isLoaded) return;

//     //     //Generate GPU handle for all entities and load into batch
//     //     for(size_t entity = 0; entity < chunk.entities.size(); entity++){

           
//     //         std::cout << "Unloading entity" << std::endl;
//     //         EntityDrawInfo drawinfo = chunk.gpuHandles[chunk.entityToHandleIndex[entity]];//gpuHandles[entityToHandleIndex[e]];
//     //         GPUMemoryHandle handle = drawinfo.handle;
//     //         //batch->Unload(handle);
            

//     //     }

//     //     //Set Chunk to active
//     //     chunk.isLoaded = false;
        
//     // };
// };