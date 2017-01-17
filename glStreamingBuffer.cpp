#include <cstring>
#include "glStreamingBuffer.h"

GLStreamingBuffer::GLStreamingBuffer()
        : currentOffset(0)
{}

GLStreamingBuffer::~GLStreamingBuffer()
{}

void GLStreamingBuffer::Update(const void* data, size_t size)
{
#ifndef NDEBUG
    if(size > this->size)
    {
        Logger::LogLine(LOG_TYPE::WARNING, "Size too big when updating buffer: , size, " > ", this->size. Only enough data to fit in the buffer will be copied");
        size = this->size;
    }
#endif // NDEBUG

    GLBufferLock(this);

    // Orphan if needed
    if(currentOffset + size > this->size)
        glBufferData(bindType, this->size, nullptr, GL_STREAM_DRAW);

    void* mappedBuffer = glMapBufferRange(bindType, currentOffset, size, GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_WRITE_BIT);
    memcpy(mappedBuffer, data, size);
    glUnmapBuffer(bindType);
}
