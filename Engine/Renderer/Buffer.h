#pragma once
#include <GL/glew.h>
#include "BufferHeap.h"
#include <iostream>
#include <vector>

class Buffer {
protected:
    unsigned int m_RendererID = 0;
    Heap heap;

    virtual GLenum GetBufferType() const = 0;
    virtual const char* GetBufferName() const = 0;

public:
    Buffer(size_t heapSize) : heap(heapSize), m_RendererID(0) {}
    
    virtual ~Buffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    virtual void Bind() const = 0;
    virtual void UnBind() const = 0;

    size_t GetSize() const { return heap.allocatedSize; }

    void Free(size_t offset) {
        heap.Free(offset);
    }

    virtual void AddData(size_t offset, size_t size, const void* data) {
        Bind();
        glBufferSubData(GetBufferType(), offset, size, data);
    }

    virtual MemoryBlock& AddData(size_t size, const void* data) {
        
        bool cheapGPUProblem = false;
        
        MemoryBlock& block = heap.Allocate(size);

        if (block.size == 0) {

            if(Resize(heap.allocatedSize * 2)){
                block = heap.Allocate(size);

            }

            else{
                return block;
            }
            
        }
        
        Bind();
        glBufferSubData(GetBufferType(), block.offset, size, data);
        return block;
    }

    virtual bool Resize(size_t newSize) {
        size_t m_Size = GetSize();
        if (newSize == m_Size) return true;

        Bind();
        
        // Only copy old data if buffer has data
        std::vector<char> oldData;
        if (m_Size > 0) {
            oldData.resize(m_Size);
            glGetBufferSubData(GetBufferType(), 0, m_Size, oldData.data());
        }

        glBufferData(GetBufferType(), newSize, nullptr, GL_DYNAMIC_DRAW);
        
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cout << GetBufferName() << ": GPU allocation failed with error " << error << std::endl;
            return false;
        }
        
        // Only restore old data if there was any
        if (m_Size > 0) {
            glBufferSubData(GetBufferType(), 0, m_Size, oldData.data());
        }

        
        heap.blocks.push_back(MemoryBlock{heap.allocatedSize, newSize - heap.allocatedSize, true});
        heap.totalSize = newSize;

        std::cout << GetBufferName() << ": Resized buffer to " << newSize << " bytes from " << m_Size << " bytes." << std::endl;
        return true;
    }

    void GetDebugInfo(bool displayBlocks = true) const {
        std::cout << "Buffer Name: " << GetBufferName() << std::endl;
        std::cout << "Buffer Size: " << GetSize() << " bytes" << std::endl;
        std::cout << "Heap Total Size: " << heap.totalSize << " bytes" << std::endl;
        std::cout << "Heap Allocated Size: " << heap.allocatedSize << " bytes" << std::endl;
        std::cout << "Memory Blocks:" << std::endl;

        if(!displayBlocks){
            return;
        }
            
        for (const auto& block : heap.blocks) {
            std::cout << "  Offset: " << block.offset 
                      << ", Size: " << block.size 
                      << ", Free: " << (block.free ? "Yes" : "No") << std::endl;
        }
    }
};
