#pragma once
#include <GL/glew.h>
#include "BufferHeap.h"
#include <iostream>


class VertexBuffer {
private:
    unsigned int m_RendererID;
    Heap heap;


public:
    VertexBuffer(const void* data, unsigned int size);

    VertexBuffer();

    ~VertexBuffer();

    void Bind() const;

    MemoryBlock& AddData(size_t size, const void* data);
    void AddData(size_t offset, size_t size, const void* data);
    void UnBind() const;

};