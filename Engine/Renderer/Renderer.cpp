#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include "BufferedBatch.h"
#include "Batch.h"
#include "Renderer.h"
#include "../Components/MeshComponent.h"




glm::mat4 projection = glm::perspective(glm::radians(90.0f), 800.0f / 600.0f, 0.1f, 100.0f);
glm::mat4 view;

//std::unordered_map<Material, Batch*> batches;

std::unique_ptr<BufferedBatch> bufferedBatch;

std::unique_ptr<Batch> batch;

glm::vec3 lightPosition = glm::vec3(0.0f, 0.0f, 3.0f);
float ambient = 0.2;

Renderer::Renderer(){
    
}

Renderer::~Renderer(){
    
}

void Renderer::CreateBatch(Shader shader, VertexBufferLayout layout){
    bufferedBatch = std::make_unique<BufferedBatch>(shader);
    bufferedBatch->AddLayout(layout);
}


void Renderer::Draw(){
   
    
        bufferedBatch->shader.Use();
        bufferedBatch->shader.SetMat4("view", glm::value_ptr(view), GL_FALSE); //Set the view matrix
        bufferedBatch->shader.SetMat4("projection", glm::value_ptr(projection), GL_FALSE); //Set projection matrix

        //Change this later
        //bind lightning
        bufferedBatch->shader.SetVec3("lightPos", glm::value_ptr(lightPosition));
        bufferedBatch->shader.SetFloat("ambient", ambient);


        bufferedBatch->Bind();
        bufferedBatch->Draw();
}

void Renderer::SetProjection(glm::mat4 mat)
{
    
}

void Renderer::SetCamera(glm::mat4 CameraMatrix)
{
    view = CameraMatrix;
}

void Renderer::Remove(MeshComponent &e)
{
    batch->RemoveMesh(e.batchTransformIdx);
}

void Renderer::UpdateTransform(glm::mat4 &t)
{
    std::cout << "Adding mesh" << std::endl;
    //Add mesh first because the ssbo use the transform as instanceindex
    

    //e.batchTransformIndex = batch->AddTransform(t);

    batch->UpdateCommandBuffer();
}


// void Renderer::AddEntity(Entity& e)  {

//     std::cout << "Adding mesh" << std::endl;
//     //Add mesh first because the ssbo use the transform as instanceindex
//     //batch->entities.push_back(e.GetHandle());
//     batch->Append(e);
//     //batch->AddMesh(e.GetComponent<MeshComponent>().MeshComponent);

//     //e.batchTransformIndex = batch->AddTransform(e.GetComponent<TransformComponent>().GetCombined());

//     batch->UpdateCommandBuffer();
    
// }

// void Renderer::HandleMeshChanges(const std::vector<std::unique_ptr<Entity>>& entities)
// {
//     batch->ClearAndRebuild(entities);
// }

// void Renderer::UpdateTransform(Entity& e){
//     glm::mat4 newMatrix = e.GetComponent<TransformComponent>().GetCombined();
//     batch->transforms.at(e.GetComponent<MeshComponent>().batchTransformIdx) = newMatrix;
// }