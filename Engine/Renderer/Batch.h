#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "RenderTypes.h"
#include "../Shader/Shader.h"

#include <memory>



struct DrawElementsIndirectCommand {
    uint32_t count;         // index count
    uint32_t instanceCount; // usually 1
    uint32_t firstIndex;    // offset in IBO
    uint32_t baseVertex;    // offset in VBO
    uint32_t baseInstance;  // usually 0
};

class Batch{
public:
    size_t vertexOffset = 0;
    size_t indexOffset = 0;

    const size_t vertexBufferSize = 10240 * 10240; //100 mb
    const size_t indexBufferSize = 10240 * 10240;

    std::vector<glm::mat4> transforms;

    Shader shader;
    
    VertexArray va;
    VertexBuffer vb;
    IndexBuffer ib;

    //For gldrawindirect
    unsigned int indBuffer;
    std::vector<DrawElementsIndirectCommand> commands;

    //for ssbo
    unsigned int ssbo = 0;
    

    /*
        TODO

        Add better memory management for the meshes
        Each mesh should have a fixed max size at the start and the buffers should be independent
    
    */


    Batch(Shader shader);
    ~Batch();


    int AddTransform(glm::mat4 t);
    void AddMesh(Mesh mesh);
    void RemoveMesh(int index);
    void AddLayout(const VertexBufferLayout& layout);
    void Bind();
    void Draw();
    void ClearBufferedData();
    void UpdateCommandBuffer();
    

    //TODO - this is just for prototyping, later use more efficient way
    //void ClearAndRebuild(const std::vector<std::unique_ptr<Entity>>& entities);

};