#include <cstring>
#include "glBuffer.h"
#include "glBufferBase.h"

GLBuffer::GLBuffer()
{}

GLBuffer::~GLBuffer()
{}

void GLBuffer::Update(const void* data, size_t size)
{
#ifndef NDEBUG
    if(IsBound())
    {
        Logger::LogLine(LOG_TYPE::WARNING, "Trying to update bound buffer, no action will be taken");
        return;
    }

    if(size > this->size)
    {
        Logger::LogLine(LOG_TYPE::WARNING, "Size too big when updating buffer: , size, " > ", this->size. Only enough data to fit in the buffer will be copied");
        size = this->size;
    }
#endif // NDEBUG

    GLBufferLock lock(this);

    void* mappedBuffer = glMapBuffer(bindType, GL_WRITE_ONLY);
    memcpy(mappedBuffer, data, size);
    glUnmapBuffer(bindType);
}