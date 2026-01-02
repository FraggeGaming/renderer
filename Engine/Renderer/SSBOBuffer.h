#pragma once
#include <GL/glew.h>
#include "BufferHeap.h"
#include <iostream>

class SSBOBuffer {
private:
    unsigned int m_RendererID;
    Heap heap;
    size_t m_Size = 0;

public:
    SSBOBuffer(const void* data, unsigned int size);

    SSBOBuffer();

    ~SSBOBuffer();

    void Bind() const;
    MemoryBlock& AddData(size_t size, const void* data);
    void UnBind() const;
    void OverWrite(size_t offset, size_t size, const void *data);
    // Resize SSBO to newSize bytes, preserving contents
    void Resize(size_t newSize);
    // Current GPU buffer size in bytes
    size_t GetSize() const { return m_Size; }
};