#pragma once
#include <GL/glew.h>
#include "BufferHeap.h"
#include <iostream>
#include <vector>

class Buffer {
protected:
    unsigned int m_RendererID = 0;
    Heap heap;
    size_t m_Size = 0;

    virtual GLenum GetBufferType() const = 0;
    virtual const char* GetBufferName() const = 0;

public:
    Buffer(size_t heapSize) : heap(heapSize), m_RendererID(0) {}
    
    virtual ~Buffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    virtual void Bind() const = 0;
    virtual void UnBind() const = 0;

    size_t GetSize() const { return m_Size; }

    void Free(size_t offset) {
        heap.Free(offset);
    }

    virtual void AddData(size_t offset, size_t size, const void* data) {
        Bind();
        glBufferSubData(GetBufferType(), offset, size, data);
    }

    virtual MemoryBlock& AddData(size_t size, const void* data) {
        MemoryBlock& block = heap.Allocate(size);
        if (block.size == 0) {
            std::cout << "Buffer: Out of memory!" << std::endl;
            return block;
        }
        Bind();
        glBufferSubData(GetBufferType(), block.offset, size, data);
        return block;
    }

    virtual void Resize(size_t newSize) {
        if (newSize == m_Size) return;

        Bind();
        std::vector<char> oldData(m_Size);
        glGetBufferSubData(GetBufferType(), 0, m_Size, oldData.data());

        glBufferData(GetBufferType(), newSize, nullptr, GL_DYNAMIC_DRAW);
        glBufferSubData(GetBufferType(), 0, m_Size, oldData.data());

        m_Size = newSize;
        heap.blocks.push_back(MemoryBlock{heap.allocatedSize, newSize - heap.allocatedSize, true});
        heap.totalSize = newSize;
    }

    void GetDebugInfo() const {
        std::cout << "Buffer Name: " << GetBufferName() << std::endl;
        std::cout << "Buffer Size: " << m_Size << " bytes" << std::endl;
        std::cout << "Heap Total Size: " << heap.totalSize << " bytes" << std::endl;
        std::cout << "Heap Allocated Size: " << heap.allocatedSize << " bytes" << std::endl;
        std::cout << "Memory Blocks:" << std::endl;
        for (const auto& block : heap.blocks) {
            std::cout << "  Offset: " << block.offset 
                      << ", Size: " << block.size 
                      << ", Free: " << (block.free ? "Yes" : "No") << std::endl;
        }
    }
};
