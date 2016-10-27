#include "glIndexBuffer.h"

GLIndexBuffer::GLIndexBuffer()
        : indiciesCount(0)
{}

GLIndexBuffer::~GLIndexBuffer()
{}

GLsizei GLIndexBuffer::GetIndiciesCount() const
{
    return indiciesCount;
}