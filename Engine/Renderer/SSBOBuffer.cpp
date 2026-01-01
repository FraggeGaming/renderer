#include "SSBOBuffer.h"

SSBOBuffer::SSBOBuffer(const void *data, unsigned int size) : heap(size)
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_COPY);
}

SSBOBuffer::~SSBOBuffer() {
    glDeleteBuffers(1, &m_RendererID);
}

SSBOBuffer::SSBOBuffer() : heap(1024 * 1024) //1 MB default
{
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1024 * 1024, nullptr, GL_DYNAMIC_COPY);
}

void SSBOBuffer::Bind() const
{
    // Bind buffer and assign to binding point 3 so shaders using
    // "layout(..., binding = 3)" can access this SSBO.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, m_RendererID);

}

void SSBOBuffer::UnBind() const
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

MemoryBlock &SSBOBuffer::AddData(size_t size, const void *data)
{
    
    MemoryBlock& block = heap.Allocate(size);
    if (block.size == 0){
        std::cout << "VertexBuffer: Out of memory!" << std::endl;
        return block;
    }

    Bind();
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, block.offset, size, data);
    return block;
}
