#include "vertexBuffer.h"

VertexBuffer::VertexBuffer()
{}

VertexBuffer::~VertexBuffer()
{}

bool VertexBuffer::Init(GLEnums::BUFFER_USAGE usage, void* data, size_t dataSize, bool memcpyData)
{
    return GLBuffer::Init(GLEnums::BUFFER_TYPE::VERTEX, usage, data, dataSize, memcpyData);
}

