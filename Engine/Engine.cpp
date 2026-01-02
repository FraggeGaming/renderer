#include "Engine.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static void MouseControlledCamera_callback(GLFWwindow* window, double xpos, double ypos){
    AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));

    ctx->camera->mouse.CalculateMouseOffset(xpos, ypos);
    MouseController m =  ctx->camera->mouse;
    glm::vec3 lookDir;
    lookDir.x = cos(glm::radians(m.yaw)) * cos(glm::radians(m.pitch));
    lookDir.y =  sin(glm::radians(m.pitch));
    lookDir.z = sin(glm::radians(m.yaw)) * cos(glm::radians(m.pitch));
    ctx->camera->m_cameraFront = glm::normalize(lookDir);
}

Engine::Engine(){
    ecs = std::make_unique<ECS>();
    systems.reserve(10);
}

void Engine::AddSystem(std::unique_ptr<System> system)
{
    system->id = 0;
    system->Init(*ecs, *this);

    systems.push_back(std::move(system));
}

void Engine::CreateWindow()
{
    if(!glfwInit()){
        std::cerr << "faild to init GLFW";
        return;
    }


    std::cout << "Requesting OpenGL 4.6 core profile..." << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Computer Graphics", nullptr, nullptr);
    if(!window){
        std::cerr << "Failed to create window";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window);
    std::cout << "GL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Ensure modern OpenGL functions are exposed by GLEW on core profiles
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK){
        std::cerr << "Failed to init GLEW";

        return;
    }   

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
}

void Engine::InitModalities()
{
    // Creates the context that holds the input and camera
    ctx = std::make_unique<AppContext>();
    input = InputHandler();
    input.SetWindow(window);

    ctx->camera = &camera;
    ctx->input = &input;

    glfwSetWindowUserPointer(window, ctx.get());
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, MouseControlledCamera_callback);
}


void Engine::AddRenderingType(Shader shader, VertexBufferLayout layout)
{
    //renderer.CreateBatch(shader, layout);
}



void Engine::Run()
{
    //Used to calculate DeltaTime
    auto prevTime = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < systems.size(); i++)
        {
            systems.at(i)->Start();
        }

    while(!glfwWindowShouldClose(window)){

        glfwPollEvents();

        //Calculate delta time
        auto present = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration_cast<std::chrono::duration<float>>(present - prevTime).count();
        prevTime = present;

        //Had to clamp this and it works, dont know why the deltatime is acting this weird. prob rounding errors
        if (dt < 0.00001f) {
            dt = 0.016666f; 
            std::cout << "WARNING: DT clamped to 0.016666" << std::endl;
        }
        // UPDATE
        input.Update(dt);

        for (size_t i = 0; i < systems.size(); i++)
        {
            systems.at(i)->Update(dt);
        }
        
        // RENDER
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (size_t i = 0; i < systems.size(); i++)
        {
            systems.at(i)->Render();
        }

        glfwSwapBuffers(window);

        //Deletes all the flaged entities
        //DeleteFlaged();
        
    }

    glfwDestroyWindow(window);
    glfwTerminate();

}

void Engine::Update(float dt)
{
        //Update stage
    

    //Preprocess stage

    //Later make the components them self trigger this so we dont need to iterate everything

    // bool rebuildMeshes = false; //only for prototyping

    // for(auto& entity : entities){
    //     if (entity->isDirty)
    //     {
    //         //Iterate all components, to see if they are dirty or needs update
    //         if (entity->hasMesh && entity->dirtyMesh)
    //         {
    //             //Remove the mesh then add it again
    //             std::cout << "Entity has dirty mesh" << std::endl;
    //             //renderer.Remove(*entity);
    //             //renderer.AddEntity(*entity);
    //             rebuildMeshes = true;

    //         }

    //         if (entity->batchTransformIndex != -1 && entity->dirtyTransform)
    //         {
    //             //Update the transform
    //             renderer.UpdateTransform(*entity);

    //         }


    //         entity->dirtyMesh = false;
    //         entity->dirtyTransform = false;
    //         entity->FlagForUpdate(false);
    //     }
    // }

    // if(rebuildMeshes){
    //     std::cout << "Rebuild the batch!" << std::endl;
    //     renderer.HandleMeshChanges(entities);
    // }
}

void Engine::Render()
{
    // glClearColor(0.0, 0.0, 0.0, 1.0);
    // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // renderer.SetCamera(ctx->camera->GetView());
    // renderer.Draw();

    // glfwSwapBuffers(window);
}

// void Engine::DeleteFlaged()
// {
//     if(!findAndDestroyEntities)
//         return;

    
//     findAndDestroyEntities = false;

//     auto it = entities.begin();

//     while(it != entities.end()){
//         const std::unique_ptr<Entity>& ent_ptr = *it;

//         if(ent_ptr->deleteFlag){
//             it = entities.erase(it);

//             //Remove from each system that has the entity
//             //Renderer etc

//             //for now, we just recreate the whole batch each time
//             std::cout << "entity deleted" << std::endl;
//         }

//         else {
//             ++it;
//         }
//     }


//     //So at the end then we cleared everything, Reinstantiate the batch

//     renderer.HandleMeshChanges(entities);
// }

// Entity* Engine::CreateEntity(){
//     std::unique_ptr<Entity> ent = std::make_unique<Entity>();
//     Entity* ent_ptr = ent.get();
//     AddEntity(std::move(ent));

//     return ent_ptr;
// }

// void Engine::AddEntity(std::unique_ptr<Entity> entity)
// {
//     entities.push_back(std::move(entity));

//     //renderer.AddEntity(entity);
// }

// void Engine::DeleteEntity(Entity& entity)
// {
//     entity.Kill();
//     findAndDestroyEntities = true;
// }



