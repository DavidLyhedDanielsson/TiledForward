#include "glVertexBuffer.h"

GLVertexBuffer::GLVertexBuffer()
{}

GLVertexBuffer::~GLVertexBuffer()
{}

GLsizei GLVertexBuffer::GetStride() const
{
    return stride;
}

const std::vector<size_t>& GLVertexBuffer::GetOffsets() const
{
    return offsets;
}
