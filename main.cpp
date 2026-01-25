#include <iostream>
#include <string>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Engine/Renderer/VertexBufferLayout.h"
#include "Engine/Shader/Shader.h"
#pragma once

#include "Engine/Engine.h"

#include "Engine/ECS/Systems/Renderer.h"
#include "Engine/ECS/Systems/Chunker.h"

#include "Game.h"

struct Player{

};


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

    engine.AddSystem(std::make_unique<Game>())->name =  "GameSystem";

    engine.AddSystem(std::make_unique<Renderer>(shader, layout))->name = "Renderer";
    engine.AddSystem(std::make_unique<Chunker>()) -> name = "Chunk Manager";

    engine.Run();

}


int main(){
    int w_height = 800;
    int w_width = 800;

    TestEngine();

}