#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Math/Math.h"
//#include "Engine/Renderer/Batch.h"
#include "Engine/Renderer/VertexBufferLayout.h"
#include "Engine/Shader/Shader.h"
#include "ObjReader.h"
#include "Engine/AppContext.h"
//#include "Engine/Renderer/Renderer.h"
#pragma once

#include "Engine/Engine.h"
#include "Engine/ECS/Systems/BatchedRenderer.h"
#include "Engine/Components/BufferedMesh.h"
#include "Engine/Components/TransformComponent.h"

struct Player{

};
// void framebuffer_size_callback(GLFWwindow* window, int width, int height);


// static void MouseControlledCamera_callback(GLFWwindow* window, double xpos, double ypos){
//     AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));


//     ctx->camera->mouse.CalculateMouseOffset(xpos, ypos);
//     MouseController m =  ctx->camera->mouse;
//     glm::vec3 lookDir;
//     lookDir.x = cos(glm::radians(m.yaw)) * cos(glm::radians(m.pitch));
//     lookDir.y =  sin(glm::radians(m.pitch));
//     lookDir.z = sin(glm::radians(m.yaw)) * cos(glm::radians(m.pitch));0
//     ctx->camera->m_cameraFront = glm::normalize(lookDir);
// }

void CreateController(Engine& engine, Entity e_ptr){
    float rotateSpeed = 2.0f;
    float speed = 0.5f;

    AppContext& ctx = *engine.ctx;

    InputHandler* input_ptr = ctx.input;


    
    float cameraSpeed = 2.0f;

    Camera* cam_ptr = ctx.camera;

    // ctx.input->BindKey(GLFW_KEY_UP,KEY_PRESS_TYPE::CONT ,[&](float dt){
        
    //     transform.AddRotation(glm::vec3(-rotateSpeed * dt,0 , 0));
        
    //     e_ptr->FlagForUpdate(true);
        
    // });

     //Rotate arround y negaitve
     ctx.input->BindKey(GLFW_KEY_DOWN,KEY_PRESS_TYPE::CONT ,[&engine, e_ptr, rotateSpeed](float dt){
        TransformComponent& transform = engine.ecs->GetComponent<TransformComponent>(e_ptr);
        transform.AddRotation(glm::vec3(rotateSpeed * dt,0 , 0));

     });



    // //Rotate x negaitve
    // ctx.input->BindKey(GLFW_KEY_LEFT,KEY_PRESS_TYPE::CONT ,[&](float dt){
    //     e_ptr->transform.AddRotation(glm::vec3(0, -rotateSpeed * dt, 0));
    //     e_ptr->dirtyTransform = true;
    //     e_ptr->FlagForUpdate(true);
    // });



    // //Rotate x
    // ctx.input->BindKey(GLFW_KEY_RIGHT,KEY_PRESS_TYPE::CONT ,[&](float dt){
    //     e_ptr->transform.AddRotation(glm::vec3(0, rotateSpeed * dt, 0));
    //     e_ptr->dirtyTransform = true;
    //     e_ptr->FlagForUpdate(true);
    // });



    // //Translate -x
    // ctx.input->BindKey(GLFW_KEY_Z,KEY_PRESS_TYPE::CONT ,[&](float dt){
    //     e_ptr->transform.Translate(-speed* dt, 0, 0);
    //     e_ptr->dirtyTransform = true;
    //     e_ptr->FlagForUpdate(true);
    // });



    // //Translate x
    // ctx.input->BindKey(GLFW_KEY_X,KEY_PRESS_TYPE::CONT ,[&](float dt){
    //     e_ptr->transform.Translate(speed* dt, 0, 0);
    //     std::cout << "---TRANSLATING---" << std::endl;
    //     std::cout << "ID: " << e_ptr->ID << std::endl;
    //     std::cout << "Pos XYZ: " << e_ptr->transform.position.x << ", " 
    //       << e_ptr->transform.position.y << ", " 
    //       << e_ptr->transform.position.z << std::endl;

    //       std::cout << "" << std::endl;
    //     e_ptr->dirtyTransform = true;
    //     e_ptr->FlagForUpdate(true);
    // });



    // //Scale +
    // ctx.input->BindKey(GLFW_KEY_L,KEY_PRESS_TYPE::CONT ,[&](float dt){
    //     e_ptr->transform.Scale(1+speed* dt, 1+speed* dt, 1+speed* dt);
    //     e_ptr->dirtyTransform = true;
    //     e_ptr->FlagForUpdate(true);
    // });

    // //Scale -
    // ctx.input->BindKey(GLFW_KEY_J,KEY_PRESS_TYPE::CONT ,[&](float dt){
    //     e_ptr->transform.Scale(1-speed* dt, 1-speed* dt, 1-speed* dt);
    //     e_ptr->dirtyTransform = true;
    //     e_ptr->FlagForUpdate(true);
    // });

    

    ctx.input->BindKey(GLFW_KEY_W,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::FORWARD);
    });

    ctx.input->BindKey(GLFW_KEY_S,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::BACK);
    });

    ctx.input->BindKey(GLFW_KEY_A,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::LEFT);
    });

    ctx.input->BindKey(GLFW_KEY_D,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::RIGHT);
    });

    ctx.input->BindKey(GLFW_KEY_E,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::UP);
    });

    ctx.input->BindKey(GLFW_KEY_Q,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::DOWN);
    });

}

void TestEngine(){

    

    
    Engine engine = Engine();

    engine.CreateWindow();

    const GLubyte* renderer = glGetString(GL_RENDERER); // GPU name
    const GLubyte* vendor = glGetString(GL_VENDOR);     // NVIDIA/AMD/Intel
    const GLubyte* version = glGetString(GL_VERSION);   // OpenGL version

    std::cout << "GPU: " << renderer << std::endl;
    std::cout << "Vendor: " << vendor << std::endl;
    std::cout << "Version: " << version << std::endl;

    engine.InitModalities();

    ShaderSrc source = Shader::ParseShader("Res/Shaders/Standard.shader");
    //std::cout << source.VertexSrc << std::endl;
    Shader shader = Shader(source.VertexSrc, source.FragmentSrc);
    shader.Use();
    
    //Define the vertexarray layout
    VertexBufferLayout layout;
    layout.Push<float>(3);  //First position of the vertex
    layout.Push<float>(2);  //texcord
    layout.Push<float>(3);  //normal
    layout.Push<float>(4);  //color

    engine.AddSystem(std::make_unique<BatchedRenderer>(shader, layout));

    //engine.AddRenderingType(shader, layout);


    //OBJ reader
    ObjReader reader = ObjReader();
    reader.SetReadParam(ObjReader::VERTEX, true);
    reader.SetReadParam(ObjReader::TEXCORD, true);
    reader.SetReadParam(ObjReader::NORMAL, true);

    AssetManager& assetManager = engine.assetManager;

    Mesh mesh  = reader.ReadObject("Res/ObjFiles/OBJ_Files/suzanne.obj");
    int mid = assetManager.Add(mesh);
    
    std::cout << "Wow" << std::endl;

    {
        Entity e = engine.ecs->CreateEntity();
        BufferedMesh m = BufferedMesh();
        m.meshID = mid;

        engine.ecs->AddComponent(e, m);

        TransformComponent transform = TransformComponent();
        engine.ecs->AddComponent(e, transform);
        CreateController(engine, e);

    }

    Mesh me  = reader.ReadObject("Res/ObjFiles/OBJ_Files/pokeball.obj");
    int meid = assetManager.Add(me);


    for (size_t i = 0; i < 100; i++)
    {
        for (size_t j = 0; j < 200; j++)
        {
            Entity e = engine.ecs->CreateEntity();
            BufferedMesh m = BufferedMesh();
            m.meshID = mid;

            engine.ecs->AddComponent(e, m);

            TransformComponent transform = TransformComponent();
            transform.Translate(2 + i*2, 0, 2 + j * 2 );
            engine.ecs->AddComponent(e, transform);
        }
    }
    
    


    //Set input

    Engine* engine_ptr = &engine;
    ObjReader* reader_ptr = &reader;
    engine.ctx->input->BindKey(GLFW_KEY_O,KEY_PRESS_TYPE::TAP ,[engine_ptr, reader_ptr](float dt){
        std::string name;
        std::string filePath = "Res/ObjFiles/OBJ_Files/";

        std::cin >> name;

        filePath.append(name);
        Mesh mesh2 = reader_ptr->ReadObject(filePath);

        
        // //batch.ClearBufferedData();
        // for (auto& ent_ptr : engine_ptr->entities)
        // {
        //     engine_ptr->DeleteEntity(*ent_ptr);
        //     std::cout << "Kill entity" << std::endl;
        // }
        
        // //batch.AddMesh(mesh);
        // Entity* e1 = engine_ptr->CreateEntity();
        // e1->AddMesh(mesh2);

        //CreateController(*engine_ptr->ctx, engine_ptr->entities, 0);
    });

    engine.Run();

}


int main(){
    int w_height = 800;
    int w_width = 800;

    TestEngine();

}