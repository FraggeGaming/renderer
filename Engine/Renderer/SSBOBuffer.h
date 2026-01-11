#pragma once
#include "Buffer.h"

class SSBOBuffer : public Buffer {
protected:
    GLenum GetBufferType() const override { return GL_SHADER_STORAGE_BUFFER; }
    const char* GetBufferName() const override { return "SSBOBuffer"; }
    unsigned int m_BindingPoint = 0;

public:
    SSBOBuffer(const void* data, unsigned int size);
    SSBOBuffer();

    void SetBinding(unsigned int binding){
        m_BindingPoint = binding;
    }
    void Bind() const override;
    void UnBind() const override;
};