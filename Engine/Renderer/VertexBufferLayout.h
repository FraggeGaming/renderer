#pragma once
#include <vector>
#include <GL/glew.h>

struct VertexBufferElement{
    unsigned int type;
    unsigned int count;
    unsigned char normalized;

    static unsigned int GetTypeSize(unsigned int type){
        switch (type)
        {
            case GL_FLOAT:
                return 4;
            case GL_UNSIGNED_INT:
                return 4;
            case GL_UNSIGNED_BYTE:
                return 1;
        }

        return 0;
    }
};

class VertexBufferLayout{
private:
    std::vector<VertexBufferElement> m_Elements;
    unsigned int m_Stride;

public:
        VertexBufferLayout() : m_Stride(0) {};

    template<typename t>
    void Push(unsigned int count){
        struct always_false : std::false_type {};
        static_assert(always_false::value, "Unsupported vertex attribute type");
    }

    

    inline const std::vector<VertexBufferElement> GetElements() const{
        return m_Elements;
    }

    //Returns the stride to the next vertex
    inline unsigned int GetStride() const {
        return m_Stride;
    }
    

};

//For each set of variables we push, we increase the stride by the type * the count
template<>
inline void VertexBufferLayout::Push<float> (unsigned int count){
    m_Elements.push_back({GL_FLOAT, count, GL_FALSE});
    m_Stride += count * VertexBufferElement::GetTypeSize(GL_FLOAT);
}

template<>
inline void VertexBufferLayout::Push<unsigned int> (unsigned int count){
    m_Elements.push_back({GL_UNSIGNED_INT, count, GL_FALSE});
    m_Stride += count *VertexBufferElement::GetTypeSize(GL_UNSIGNED_INT);
}



template<>
inline void VertexBufferLayout::Push<unsigned char> (unsigned int count){
    m_Elements.push_back({GL_UNSIGNED_BYTE, count, GL_TRUE});
    m_Stride += count *VertexBufferElement::GetTypeSize(GL_UNSIGNED_BYTE);

}