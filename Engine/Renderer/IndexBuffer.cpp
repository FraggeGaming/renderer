#include "IndexBuffer.h"
#include <vector>


IndexBuffer::IndexBuffer(const void* data, unsigned int count) : m_Count(count), heap(count * sizeof(unsigned int)){
    glGenBuffers(1, &m_RendererID);
    Bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count*sizeof(unsigned int), data, GL_STATIC_DRAW);
    m_Size = count * sizeof(unsigned int);

}

IndexBuffer::~IndexBuffer(){
    glDeleteBuffers(1, &m_RendererID);
}

void IndexBuffer::AddData(size_t offset, size_t size, const void* data){
    m_Count += size / sizeof(unsigned int);
    Bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
}

MemoryBlock& IndexBuffer::AddData( size_t size, const void* data){
    MemoryBlock& block = heap.Allocate(size);

    if (block.size == 0){
        std::cout << "IndexBuffer: Out of memory!" << std::endl;
        return block;
    }

    m_Count += size / sizeof(unsigned int);
    Bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, block.offset, size, data);

    return block;
}

void IndexBuffer::Resize(size_t newSize)
{
    if (newSize == m_Size) return;

    Bind();
    std::vector<char> oldData(m_Size);
    glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Size, oldData.data());

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, m_Size, oldData.data());

    m_Size = newSize;
    heap.blocks.push_back(MemoryBlock{heap.allocatedSize, newSize - heap.allocatedSize, true});
    heap.totalSize = newSize;
}

void IndexBuffer::Bind() const{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}
void IndexBuffer::UnBind() const{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::Clear(){
    m_Count = 0;
}
