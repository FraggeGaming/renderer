#pragma once
#include "Buffer.h"

class VertexBuffer : public Buffer {
protected:
    GLenum GetBufferType() const override { return GL_ARRAY_BUFFER; }
    const char* GetBufferName() const override { return "VertexBuffer"; }

public:
    VertexBuffer(const void* data, unsigned int size);
    VertexBuffer(unsigned int size);
    VertexBuffer();

    void Bind() const override;
    void UnBind() const override;
};