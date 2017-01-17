#ifndef GLSTREAMINGBUFFER_H__
#define GLSTREAMINGBUFFER_H__

#include "glBufferBase.h"

class GLStreamingBuffer
        : public GLBufferBase
{
public:
    GLStreamingBuffer();
    ~GLStreamingBuffer();

    void Update(const void* data, size_t size) override;

private:
    GLint currentOffset;
};

#endif // GLSTREAMINGBUFFER_H__
