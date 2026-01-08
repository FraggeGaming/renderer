#include "IndexBuffer.h"

IndexBuffer::IndexBuffer(const void* data, unsigned int count) 
    : Buffer(count * sizeof(unsigned int)), m_Count(count) {
    glGenBuffers(1, &m_RendererID);
    Bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * sizeof(unsigned int), data, GL_STATIC_DRAW);
    m_Size = count * sizeof(unsigned int);
}

void IndexBuffer::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
}

void IndexBuffer::UnBind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void IndexBuffer::AddData(size_t offset, size_t size, const void* data) {
    m_Count += size / sizeof(unsigned int);
    Buffer::AddData(offset, size, data);
}

MemoryBlock& IndexBuffer::AddData(size_t size, const void* data) {
    m_Count += size / sizeof(unsigned int);
    return Buffer::AddData(size, data);
}

void IndexBuffer::Clear() {
    m_Count = 0;
}
