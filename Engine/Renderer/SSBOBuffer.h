#pragma once
#include <GL/glew.h>
#include "BufferHeap.h"
#include <iostream>

class SSBOBuffer {
private:
    unsigned int m_RendererID;
    Heap heap;

public:
    SSBOBuffer(const void* data, unsigned int size);

    SSBOBuffer();

    ~SSBOBuffer();

    void Bind() const;
    MemoryBlock& AddData(size_t size, const void* data);
    void UnBind() const;
    void OverWrite(size_t offset, size_t size, const void *data);
};