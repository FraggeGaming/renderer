#include "VertexBuffer.h"
#include <vector>


VertexBuffer::VertexBuffer(const void* data, unsigned int size) : heap(size){
    glGenBuffers(1, &m_RendererID);                             //Creates buffer
    Bind();
    glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);  //Upload the data
    m_Size = size;

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

void VertexBuffer::Resize(size_t newSize)
{
    if (newSize == m_Size) return;

    Bind();
    // Read existing data
    std::vector<char> oldData(m_Size);
    glGetBufferSubData(GL_ARRAY_BUFFER, 0, m_Size, oldData.data());

    // Reallocate buffer with new size and upload old contents
    glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_Size, oldData.data());

    m_Size = newSize;
    // Inform heap about larger total size by appending a free block
    heap.blocks.push_back(MemoryBlock{heap.allocatedSize, newSize - heap.allocatedSize, true});
    heap.totalSize = newSize;
}

void VertexBuffer::UnBind() const{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
