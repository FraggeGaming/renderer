#pragma once
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <chrono>
#include <iomanip> //For debug timer
#

#include "Renderer/VertexBufferLayout.h"
#include "Shader/Shader.h"
#include "Renderer/RenderTypes.h"
#include "ECS/ECS.h"
#include "InputHandler.h"
#include "AppContext.h"
#include "ECS/Systems/System.h"

// Profiling system
class ProfileData {
public:
    struct Entry {
        std::string name;
        long long totalMicroseconds = 0;
        int callCount = 0;
        long long largestMicroseconds = 0;
        
        double getAverageMicros() const { return callCount > 0 ? (double)totalMicroseconds / callCount : 0.0; }
        double getAverageMillis() const { return getAverageMicros() / 1000.0; }
    };
    
private:
    static inline std::unordered_map<std::string, Entry> entries;
    
public:
    static void Record(const std::string& name, long long microseconds) {
        entries[name].name = name;
        entries[name].totalMicroseconds += microseconds;
        entries[name].callCount++;
        if (microseconds > entries[name].largestMicroseconds) {
            entries[name].largestMicroseconds = microseconds;
        }
    }
    
    static void PrintStats() {
        std::cout << "\n=== PROFILING STATISTICS ===" << std::endl;
        std::cout << std::left << std::setw(35) << "Function" 
                  << std::right << std::setw(12) << "Calls" 
                  << std::setw(15) << "Total (ms)" 
                  << std::setw(15) << "Avg (µs)" 
                  << std::setw(15) << "Avg (ms)"
                  << std::setw(15) << "Largest (ms)" << std::endl;
        std::cout << std::string(92, '-') << std::endl;
        
        // Sort by total time
        std::vector<Entry> sorted;
        for (const auto& [name, entry] : entries) {
            sorted.push_back(entry);
        }
        std::sort(sorted.begin(), sorted.end(), [](const Entry& a, const Entry& b) {
            return a.totalMicroseconds > b.totalMicroseconds;
        });
        
        for (const auto& entry : sorted) {
            std::cout << std::left << std::setw(35) << entry.name
                      << std::right << std::setw(12) << entry.callCount
                      << std::setw(15) << std::fixed << std::setprecision(3) << (entry.totalMicroseconds / 1000.0)
                      << std::setw(15) << std::fixed << std::setprecision(2) << entry.getAverageMicros()
                      << std::setw(15) << std::fixed << std::setprecision(3) << entry.getAverageMillis()
                      << std::setw(15) << std::fixed << std::setprecision(3) << (entry.largestMicroseconds / 1000.0)
                      << std::endl;
        }
        std::cout << "========================\n" << std::endl;
    }
    
    static void Reset() {
        entries.clear();
    }
};

class Timer {
    std::string name;
    bool silent;
    std::chrono::time_point<std::chrono::high_resolution_clock> start;
public:
    Timer(const std::string& name, bool silent = false) 
        : name(name), silent(silent), start(std::chrono::high_resolution_clock::now()) {}
    
    ~Timer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        ProfileData::Record(name, duration);
        
        if(!silent && duration > 50) {
            std::cout << name << ": " << duration << " µs (" << duration/1000.0 << " ms)" << std::endl;
        }
    }
};




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

    Mesh& Get(int id){
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
    std::unique_ptr<AssetManager> assetManager;
    //AppContext* context;

    int height = 900, width = 1200; //Add to context later?
    GLFWwindow* window;

    bool findAndDestroyEntities = false;

    Engine();

    System* AddSystem(std::unique_ptr<System> system);

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