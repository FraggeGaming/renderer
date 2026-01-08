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

    glfwSwapInterval(0); //Disable VSync

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
    // FPS tracking (print every 2 seconds)
    int fpsFrameCount = 0;
    float fpsTimer = 0.0f;
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

        // FPS counting
        fpsFrameCount++;
        fpsTimer += dt;
        if (fpsTimer >= 2.0f) {
            float fps = fpsFrameCount / fpsTimer;
            std::cout << "FPS: " << fps << " (" << fpsFrameCount << " frames in " << fpsTimer << "s)" << std::endl;
            fpsFrameCount = 0;
            fpsTimer = 0.0f;
        }

        //Had to clamp this and it works, dont know why the deltatime is acting this weird. prob rounding errors
        if (dt < 0.00001f) {
            dt = 0.016666f; 
            std::cout << "WARNING: DT clamped to 0.016666" << std::endl;
        }
        // UPDATE
        Update(dt);
        
        // RENDER
        Render();

        //Deletes all the flaged entities
        //DeleteFlaged();
        
    }

    glfwDestroyWindow(window);
    glfwTerminate();

}

void Engine::Update(float dt)
{
    input.Update(dt);

    for (size_t i = 0; i < systems.size(); i++)
    {
        systems.at(i)->Update(dt);
    }
}

void Engine::Render()
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (size_t i = 0; i < systems.size(); i++)
    {
        systems.at(i)->Render();
    }

    glfwSwapBuffers(window);
}