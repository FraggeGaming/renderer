#include "ECS.h"
#include <iostream>
#include <string>
#include <chrono>
#include <functional>


struct TestComponent : Component {
    int value = 10;
};

struct BestComponent : Component {
    bool popsmoke = true;
};

struct MComp : Component {
    int value = 9999;
};

struct TComp : Component {
    int value = 5000;
};



struct Debugger : Component {
    int parentName;
    std::string componentName = "Real Madrid";
};


template<typename F>
long long MeasureAverageTime(F&& function, int iterations = 10){
    long long total_time = 0;

    for (int i = 0; i < iterations; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        
        function();
        
        auto end = std::chrono::high_resolution_clock::now();
        total_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    return total_time / iterations;
}


//Add a view, or achetype, so we can fetch by component pairs

int main(){ 
    ECS ecs;


    for(int i = 0; i < 50000; i++){

        EntityHandle handler = ecs.CreateEntity();

        Debugger t = Debugger();
        t.parentName = handler.id;

        //Entity e = ecs.GetEntity(h);
        ecs.GetEntity(handler).AddComponent(t);
    }

    for(int i = 0; i < 50000; i++){

        EntityHandle handler = ecs.CreateEntity();

        MComp m = MComp();
        //Entity e = ecs.GetEntity(h);
        ecs.AddComponent(handler, m);
        //ecs.GetEntity(handler).AddComponent(m);
        TComp t = TComp();
        ecs.AddComponent(handler, t);
    }

    int iterations = 100;

    //Test iterate the view with all mesh and transform components
    View<MComp, TComp> view(ecs);
    auto avg_time = MeasureAverageTime([&](){
        for(auto handle : view) {
            TComp& t = ecs.GetComponent<TComp>(handle);
            MComp& m = ecs.GetComponent<MComp>(handle);
        }
    }, iterations);


    std::cout << "View Timer\n";
    std::cout << "Average: " << avg_time << " µs\n";
    double fps = 1.0 / (avg_time / 1'000'000.0);

    std::cout << "Average FPS: " << fps<< "\n";
    
    
    return 0;
}


