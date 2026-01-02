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





glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
glm::mat4 view;

glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 3.0f);
float ambient = 0.2;

bool isDirty = true;

bool isModified = false;

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
  
}

void BatchedRenderer::Remove(BufferedMesh& e)
{
    //batch->Remove(e.batchIdx);

}

void BatchedRenderer::SetProjection(glm::mat4 mat)
{
}


void BatchedRenderer::Start()
{
    ecs->view<BufferedMesh, TransformComponent>().each([&](int entityId, BufferedMesh& m, TransformComponent& t) {
        if(m.isLoaded == false){
            Mesh mesh = engine->assetManager.Get(m.meshID);
            GPUMemoryHandle handle = batch->Load(m, mesh, t.GetCombined());

            engine->ecs->AddComponent<GPUMemoryHandle>(entityId, handle);
            m.isLoaded = true;
        }
    });
}

void BatchedRenderer::Update(float dt)
{

    std::unordered_map<int, std::vector<int>> meshGroups;
    std::vector<int> lookupTable;
    std::vector<GPUMemoryHandle> finalCommands;


    ecs->view<BufferedMesh, GPUMemoryHandle, TransformComponent>().each([&](int entityId, BufferedMesh& m, GPUMemoryHandle& h, TransformComponent& t) {
        if (t.isDirty) {
            batch->UpdateTransform(h.ssboIndex, t.GetCombined());
            t.isDirty = false;
        }
        meshGroups[m.meshID].push_back(h.ssboIndex);


    });

    for (auto& [meshID, instances] : meshGroups) {
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

    batch->UpdateInstanceLookupBuffer(lookupTable);
    batch->SetDrawVector(finalCommands);
    

    /*
    std::vector<GPUMemoryHandle> drawCommands;

    ecs->view<GPUMemoryHandle, TransformComponent>().each([&](int entityId, GPUMemoryHandle& m, TransformComponent& t) {
        

        // 3. Frustum Culling (Optional but recommended)
        //if (!cam.IsVisible(transform.bounds)) continue;

        

        // 4. Update SSBO Transform if dirty
        if (t.isDirty) {
            batch->UpdateTransform(m.ssboIndex, t.GetCombined());
            t.isDirty = false;
            //std::cout << "Updated Transform in ssbo" << std::endl;
        }

        // 5. Add a command to the batch


        drawCommands.push_back(m);

    });

    
    //batch->SetDrawVector(drawCommands);

    */


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
