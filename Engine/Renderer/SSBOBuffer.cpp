#include "SSBOBuffer.h"

SSBOBuffer::SSBOBuffer(const void* data, unsigned int size) : Buffer(size) {
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_COPY);
 
}

SSBOBuffer::SSBOBuffer() : Buffer(1024 * 1024) {
    glGenBuffers(1, &m_RendererID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1024 * 1024, nullptr, GL_DYNAMIC_COPY);
}

void SSBOBuffer::Bind() const {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, m_RendererID);
}

void SSBOBuffer::UnBind() const {
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
