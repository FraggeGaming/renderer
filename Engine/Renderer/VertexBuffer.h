#pragma once
#include <GL/glew.h>
#include "BufferHeap.h"
#include <iostream>


class VertexBuffer {
private:
    unsigned int m_RendererID;
    Heap heap;
    size_t m_Size = 0;


public:
    VertexBuffer(const void* data, unsigned int size);

    VertexBuffer();

    ~VertexBuffer();

    void Bind() const;

    // Resize the GPU buffer to newSize bytes, preserving existing contents.
    void Resize(size_t newSize);
    // Current GPU buffer size in bytes
    size_t GetSize() const { return m_Size; }

    MemoryBlock& AddData(size_t size, const void* data);
    void AddData(size_t offset, size_t size, const void* data);
    void UnBind() const;

};