#include "glIndexBuffer.h"

GLIndexBuffer::GLIndexBuffer()
        : indexCount(0)
{}

GLIndexBuffer::~GLIndexBuffer()
{}

void GLIndexBuffer::Update(const std::vector<GLuint>& indicies)
{
    GLBuffer::Update(indicies.data(), indicies.size() * sizeof(GLint));

    indexCount = (GLsizei)indicies.size();
}

GLsizei GLIndexBuffer::GetIndiciesCount() const
{
    return indexCount;
}
