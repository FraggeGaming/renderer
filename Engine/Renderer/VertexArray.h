#pragma once
#include <GL/glew.h>
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

enum VertexType{
    FLOAT, INT, UNSIGNED_INT, SHORT, UNSIGNED_SHORT, DOUBLE 
};

class VertexArray {
private:
    unsigned int m_RendererID;

public:
    VertexArray();
    

    ~VertexArray();

    void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);

    void AddAttribPointer(unsigned int attrib, int count, VertexType type, GLboolean normalized, const void* pointer);

    void Bind() const;
    void UnBind() const;

};