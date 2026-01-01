#include "VertexArray.h"
#include <iostream>


VertexArray::VertexArray(){
    glGenVertexArrays(1, &m_RendererID);
    glBindVertexArray(m_RendererID);
}

VertexArray::~VertexArray(){
    glDeleteVertexArrays(1, &m_RendererID);
}

void VertexArray::AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout){
    Bind();
    vb.Bind();
    const auto& elements = layout.GetElements();
    unsigned int offset = 0;
    for(unsigned int i = 0; i< elements.size(); i++){
        const auto& element = elements[i];
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, element.count, element.type, element.normalized, layout.GetStride(),  (const void*)offset);
        //For each vertex attribute we specify using the layout, we set the offset to the pointer by the count * size
        offset += element.count*VertexBufferElement::GetTypeSize(element.type);
    }
    

}

void VertexArray::AddAttribPointer(unsigned int attrib, int count, VertexType type, GLboolean normalized, const void* pointer){

    GLenum gltype;
    GLsizei typeSize;

    switch (type)
    {
    case VertexType::FLOAT:
        gltype = GL_FLOAT;
        typeSize = sizeof(float);
        break;
    case VertexType::INT:
        gltype = GL_INT;
        typeSize = sizeof(int);
        break;
    case VertexType::UNSIGNED_INT:
        gltype = GL_UNSIGNED_INT;
        typeSize = sizeof(unsigned int);
        break;
    case VertexType::SHORT:
        gltype = GL_SHORT;
        typeSize = sizeof(short);
        break;
    case VertexType::UNSIGNED_SHORT:
        gltype = GL_UNSIGNED_SHORT;
        typeSize = sizeof(unsigned short);
        break;
    case VertexType::DOUBLE:
        gltype = GL_DOUBLE;
        typeSize = sizeof(double);
        break;
    
    default:
        std::cerr << "Unsupported GL type in AddAttribPointer\n";   
        break;
    }


    glVertexAttribPointer(attrib, count, gltype, normalized, typeSize * count, pointer);

}

void VertexArray::Bind() const{
    glBindVertexArray(m_RendererID);
}

void VertexArray::UnBind() const{
    glBindVertexArray(0);
}