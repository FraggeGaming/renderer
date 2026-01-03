#pragma once
#include <GL/glew.h>
#include "BufferHeap.h"
#include <iostream>

class IndexBuffer {
private:
    unsigned int m_RendererID;
    unsigned int m_Count;
    Heap heap;
    size_t m_Size = 0;


public:
    IndexBuffer(const void* data, unsigned int size);

    ~IndexBuffer();

    MemoryBlock& AddData(size_t size, const void* data);

    void AddData(size_t offset, size_t size, const void* data);
    void Free(size_t offset){
        heap.Free(offset);
    }

    void Bind() const;
    void UnBind() const;
    void Clear();
    inline unsigned int GetCount() const{ return m_Count;}

    // Resize the GPU index buffer to newSize bytes, preserving contents
    void Resize(size_t newSize);
    // Current GPU buffer size in bytes
    size_t GetSize() const { return m_Size; }

};