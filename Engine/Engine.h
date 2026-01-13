#pragma once
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <chrono>

#include "Renderer/VertexBufferLayout.h"
#include "Shader/Shader.h"
#include "Renderer/RenderTypes.h"
#include "ECS/ECS.h"
#include "InputHandler.h"
#include "AppContext.h"
#include "ECS/Systems/System.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);

static void MouseControlledCamera_callback(GLFWwindow* window, double xpos, double ypos);

struct AssetManager{

    std::vector<Mesh> meshStorage;

    // Key: MeshID, Value:Current Index in the vector    
    std::unordered_map<int, size_t> meshMap;

    void RemoveMesh(int meshId) {
        size_t indexToRemove = meshMap[meshId];
        
        meshStorage[indexToRemove] = meshStorage.back();

        int movedMeshId = meshStorage[indexToRemove].id;
        meshMap[movedMeshId] = indexToRemove;

        meshStorage.pop_back();
        meshMap.erase(meshId);
    }

    int Add(Mesh mesh){
        if(meshMap.find(mesh.id) == meshMap.end()){
            meshStorage.push_back(mesh);
            meshMap[mesh.id] = meshStorage.size()-1;
        }

        return mesh.id;
    }

    Mesh Get(int id){
        return meshStorage[meshMap[id]];
    }

    void Remove(int id){
        meshMap.erase(id);
    }

};


class Engine{


public:
    
    Camera camera;
    InputHandler input;
    std::unique_ptr<ECS> ecs;
    std::vector<std::unique_ptr<System>> systems;
    
    std::unique_ptr<AppContext> ctx;
    AssetManager assetManager;
    //AppContext* context;

    int height = 900, width = 1200; //Add to context later?
    GLFWwindow* window;

    bool findAndDestroyEntities = false;

    Engine();

    void AddSystem(std::unique_ptr<System> system);

    template<typename T>
    T* GetSystem(){
        static_assert(std::is_base_of<System, T>::value, "T must derive from System");

        for (auto& sys : systems)
        {
            if (auto ptr = dynamic_cast<T*>(sys.get()))
                return ptr;
        }

        return nullptr;
    }

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