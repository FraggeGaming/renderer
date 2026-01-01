#pragma once
#include "../ECS.h"
using Entity = int;

class Engine;

class System{
    public:
    Engine* engine = nullptr;
    ECS* ecs = nullptr;
    

    int id;
    System() = default;
    ~System() = default;
    void Init(ECS& ecs, Engine& engine){
        this->ecs = &ecs;
        this->engine = &engine;
    }
    virtual void Start() = 0;
    virtual void Update(float dt) = 0;

    virtual void Render() = 0;
    
};