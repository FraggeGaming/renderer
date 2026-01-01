#include "VertexBuffer.h"


VertexBuffer::VertexBuffer(const void* data, unsigned int size) : heap(size){
    glGenBuffers(1, &m_RendererID);                             //Creates buffer
    Bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);  //Upload the data

}

VertexBuffer::~VertexBuffer(){
    glDeleteBuffers(1, &m_RendererID);
}

void VertexBuffer::Bind() const{
    glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
}

void VertexBuffer::AddData(size_t offset, size_t size, const void *data)
{
    Bind();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

MemoryBlock& VertexBuffer::AddData( size_t size, const void* data){
    MemoryBlock& block = heap.Allocate(size);
    if (block.size == 0){
        std::cout << "VertexBuffer: Out of memory!" << std::endl;
        return block;
    }

    Bind();
    glBufferSubData(GL_ARRAY_BUFFER, block.offset, size, data);

    return block;
}

void VertexBuffer::UnBind() const{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
