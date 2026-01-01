#pragma once
#include <GL/glew.h>
#include "BufferHeap.h"
#include <iostream>

class IndexBuffer {
private:
    unsigned int m_RendererID;
    unsigned int m_Count;
    Heap heap;


public:
    IndexBuffer(const void* data, unsigned int size);

    ~IndexBuffer();

    MemoryBlock& AddData(size_t size, const void* data);

    void AddData(size_t offset, size_t size, const void* data);

    void Bind() const;
    void UnBind() const;
    void Clear();
    inline unsigned int GetCount() const{ return m_Count;}

};