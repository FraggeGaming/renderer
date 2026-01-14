#pragma once
#include "Buffer.h"

class IndexBuffer : public Buffer {
private:
    unsigned int m_Count;

protected:
    GLenum GetBufferType() const override { return GL_ELEMENT_ARRAY_BUFFER; }
    const char* GetBufferName() const override { return "IndexBuffer"; }

public:
    IndexBuffer(const void* data, unsigned int count);
    IndexBuffer(unsigned int count);

    void Bind() const override;
    void UnBind() const override;
    
    void AddData(size_t offset, size_t size, const void* data) override;
    MemoryBlock& AddData(size_t size, const void* data) override;

    void Clear();
    inline unsigned int GetCount() const { return m_Count; }
};