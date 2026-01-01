#pragma once
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <chrono>

#include "Renderer/VertexBufferLayout.h"
#include "Shader/Shader.h"
#include "ECS/ECS.h"
#include "Renderer/Renderer.h"
#include "InputHandler.h"
#include "AppContext.h"
#include "ECS/Systems/System.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

static void MouseControlledCamera_callback(GLFWwindow* window, double xpos, double ypos);


class Engine{


public:
    
    Camera camera;
    InputHandler input;
    std::unique_ptr<ECS> ecs;
    std::vector<std::unique_ptr<System>> systems;
    
    std::unique_ptr<AppContext> ctx;
    //AppContext* context;

    int height = 800, width = 600; //Add to context later?
    GLFWwindow* window;

    bool findAndDestroyEntities = false;

    Engine();

    void AddSystem(std::unique_ptr<System> system);


    void CreateWindow();

    void InitModalities();

    void AddRenderingType(Shader shader, VertexBufferLayout layout);

    //void AddEntity(std::unique_ptr<Entity> entity);

    //Entity* CreateEntity();

    //void DeleteEntity(Entity& entity);

    //Starts the game loop
    void Run();

    void Update(float dt);

    void Render();

    void DeleteFlaged();
};