#include "Game.h"
#include "ObjReader.h"
#include "Engine/ECS/Systems/Renderer.h"
#include "Engine/Engine.h"

void Game::CreateController(Entity e_ptr){
    float rotateSpeed = 2.0f;
    float speed = 0.5f;

    
    float cameraSpeed = 7.0f;

    Camera* cam_ptr = &engine->camera;
    Engine* engine = this->engine;
    AppContext* ctx = engine->ctx.get();

    engine->ctx->input->BindKey(GLFW_KEY_P,KEY_PRESS_TYPE::TAP ,[engine, e_ptr, rotateSpeed](float dt){
        if (auto* batchedRenderer = engine->GetSystem<Renderer>()) {
            batchedRenderer->DebugTrace();
        }
     });


     engine->ctx->input->BindKey(GLFW_KEY_ESCAPE,KEY_PRESS_TYPE::TAP ,[engine, e_ptr, rotateSpeed](float dt){
             std::exit(0);

     });

     engine->ctx->input->BindKey(GLFW_KEY_C,KEY_PRESS_TYPE::TAP ,[engine, e_ptr, rotateSpeed](float dt){
        if (auto* batchedRenderer = engine->GetSystem<Renderer>()) {
            batchedRenderer->UseChunking = !batchedRenderer->UseChunking;
            std::cout << "Renderer: UseChunking set to " << (batchedRenderer->UseChunking ? "true" : "false") << std::endl;
        }
     });

    // ctx.input->BindKey(GLFW_KEY_UP,KEY_PRESS_TYPE::CONT ,[&](float dt){
        
    //     transform.AddRotation(glm::vec3(-rotateSpeed * dt,0 , 0));
        
    //     e_ptr->FlagForUpdate(true);
        
    // });

     //Rotate arround y negaitve
     engine->ctx->input->BindKey(GLFW_KEY_DOWN,KEY_PRESS_TYPE::CONT ,[engine, e_ptr, rotateSpeed](float dt){
        TransformComponent& transform = engine->ecs->GetComponent<TransformComponent>(e_ptr);
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

    

    engine->ctx->input->BindKey(GLFW_KEY_W,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::FORWARD);
    });

    engine->ctx->input->BindKey(GLFW_KEY_S,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::BACK);
    });

    engine->ctx->input->BindKey(GLFW_KEY_A,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::LEFT);
    });

    engine->ctx->input->BindKey(GLFW_KEY_D,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::RIGHT);
    });

    engine->ctx->input->BindKey(GLFW_KEY_E,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::UP);
    });

    engine->ctx->input->BindKey(GLFW_KEY_Q,KEY_PRESS_TYPE::CONT ,[cam_ptr, cameraSpeed](float dt){
        cam_ptr->MoveFPS( cameraSpeed*dt, FPSDirection::DOWN);
    });

}

void Game::Start()
{
    //OBJ reader
    ObjReader reader = ObjReader();
    reader.SetReadParam(ObjReader::VERTEX, true);
    reader.SetReadParam(ObjReader::TEXCORD, true);
    reader.SetReadParam(ObjReader::NORMAL, true);

    AssetManager& assetManager = engine->assetManager;

    Mesh mesh  = reader.ReadObject("Res/ObjFiles/OBJ_Files/suzanne.obj");
    int mid = assetManager.Add(mesh);

    {
        Entity e = engine->ecs->CreateEntity();
        MeshCapsule m = MeshCapsule();
        m.meshID = mid;

        engine->ecs->AddComponent(e, m);

        TransformComponent transform = TransformComponent();
        engine->ecs->AddComponent(e, transform);
        CreateController(e);

    }

    Mesh me  = reader.ReadObject("Res/ObjFiles/OBJ_Files/pokeball.obj");
    int meid = assetManager.Add(me);

    Mesh me1  = reader.ReadObject("Res/ObjFiles/OBJ_Files/teddy.obj");
    int me1id = assetManager.Add(me1);

    std::cout << "Added Mesh with ID: " << mid << std::endl;


    for (size_t i = 0; i < 300; i++)
    {
        for (size_t j = 0; j < 300; j++)
        {
            Entity e = engine->ecs->CreateEntity();
            MeshCapsule m = MeshCapsule();

            if(j % 2 == 0){
                m.meshID = meid;
            }
            else if (j % 3 == 0){
                m.meshID = me1id;
            }
            else{   
                m.meshID = mid;
            }

            engine->ecs->AddComponent(e, m);

            TransformComponent transform = TransformComponent();
            transform.Translate(2 + i*2, 0, 2 + j * 2 );
            engine->ecs->AddComponent(e, transform);
        }
    }

    //Set input

    
    ObjReader* reader_ptr = &reader;
    Engine* engine = this->engine;

    engine->ctx->input->BindKey(GLFW_KEY_O,KEY_PRESS_TYPE::TAP ,[engine, reader_ptr](float dt){
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
}

void Game::Update(float dt)
{
    ecs->view<TransformComponent>().each([&](int entityId, TransformComponent& t) {
        
        t.AddRotation(glm::vec3(0.0f, 2*dt, 1*dt));
        
    });

}

void Game::Render()
{
}
