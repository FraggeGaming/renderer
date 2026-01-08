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

    engine.AddSystem(std::make_unique<Game>());   
    engine.AddSystem(std::make_unique<BatchedRenderer>(shader, layout));
    engine.Run();

}


int main(){
    int w_height = 800;
    int w_width = 800;

    TestEngine();

}