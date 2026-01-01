#include "ECS.h"

#include <iostream>
#include <string>
#include <chrono>
#include <functional>


struct TestComponent {
    int value = 10;
};

struct Player{
    int value = 67;
};

struct MComp {
    int value = 1;
};

struct TComp{
    int value = -1;
};



struct Debugger{
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


int main(){ 
    ECS ecs;


    for(int i = 0; i < 5; i++){

        Entity e = ecs.CreateEntity();

        Debugger t = Debugger();
        t.parentName = e;

        //Entity e = ecs.GetEntity(h);
        ecs.AddComponent(e, t);
    }

    Entity playerId = ecs.CreateEntity();
    Player t = Player();

    
    std::cout << "Player: " << t.value<< "\n";

    ecs.AddComponent(playerId, t);

    int entCount = 10000000;
    for(int i = 0; i < entCount; i++){

        Entity e = ecs.CreateEntity();

        MComp m = MComp();
        //Entity e = ecs.GetEntity(h);
        ecs.AddComponent(e, m);
        //ecs.GetEntity(handler).AddComponent(m);
        TComp t = TComp();
        ecs.AddComponent(e, t);
    }

    // for(int i = 500; i< 1000; i++){
    //     ecs.Remove(i);
    // }

    int iterations = 100;

    //Test iterate the view with all mesh and transform components
   
    // auto& mStore = ecs.GetStore<MComp>();
    // auto& tStore = ecs.GetStore<TComp>();
    

/*
    auto avg_time = MeasureAverageTime([&](){

        int sum = 0;

        for (int entityId : ecs.view<MComp, TComp>()) {
            // The Iterator's SkipInvalid() logic is running behind the scenes here
            auto& t = ecs.GetComponent<TComp>(entityId);
            auto& m = ecs.GetComponent<MComp>(entityId);
            sum += m.value + t.value;
        }
        
        if(sum != 0){
            std::cout << "Something went wrong!!! " << sum <<  "\n";
        }

        
        //do something with the sum
    }, iterations);


    
        


    std::cout << "View Timer with for each loop\n";
    std::cout << "Average: " << avg_time << " µs\n";
    double fps = 1.0 / (avg_time / 1'000'000.0);

    std::cout << "Average FPS: " << fps<< "\n";

    */

    auto avg_time = MeasureAverageTime([&](){

        int sum = 0;

        
        ecs.view<MComp, TComp>().each([&](MComp& m, TComp& t) {
            // Logic here
            sum += m.value + t.value;
        });
       
        if(sum != 0){
            std::cout << "Something went wrong!!! " << sum <<  "\n";
        }
        //do something with the sum
    }, iterations);


    std::cout << "View Timer with functional\n";
    std::cout << "Average: " << avg_time << " µs\n";
    double fps = 1.0 / (avg_time / 1'000'000.0);

    std::cout << "Average FPS: " << fps<< "\n";

    Player p = ecs.GetComponent<Player>(playerId);

    std::cout << "Player: " << p.value<< "\n";
    
    
    return 0;
}


