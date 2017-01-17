#include "glBufferBase.h"

#include <cstring>

GLBufferBase::GLBufferBase()
        : buffer(0)
        , bindType(0)
        , usage(0)
        , size(0)
        , isBound(false)
{ }

GLBufferBase::~GLBufferBase()
{
}

bool GLBufferBase::Init(GLEnums::BUFFER_TYPE bufferType, GLEnums::BUFFER_USAGE usage, void* data, size_t dataSize)
{
    InitInternal(bufferType, usage, data, dataSize);
}

bool GLBufferBase::InitInternal(GLEnums::BUFFER_TYPE bufferType, GLEnums::BUFFER_USAGE usage, void* data, size_t dataSize)
{
    this->bindType = (GLuint)bufferType;
    this->usage = (GLuint)usage;
    this->size = dataSize;

    glGenBuffers(1, &buffer);
    GLBufferLock lock(this);
    glBufferData(bindType, dataSize, data, this->usage);

    return false;
}

void GLBufferBase::Bind()
{
    glBindBuffer(bindType, buffer);

    isBound = true;
}

void GLBufferBase::Unbind()
{
    glBindBuffer(bindType, 0);

    isBound = false;
}

uint32_t GLBufferBase::roundToNextPower(uint32_t value) const
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value++;

    return value;
}

uint64_t GLBufferBase::roundToNextPower(uint64_t value) const
{
    value--;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    value |= value >> 32;
    value++;

    return value;
}

bool GLBufferBase::IsBound() const
{
    return isBound;
}

GLBufferLock::GLBufferLock(GLBufferBase* buffer)
    : buffer(buffer)
{
    this->buffer->Bind();
}

GLBufferLock::GLBufferLock(GLBufferBase& buffer)
        : buffer(&buffer)
{
    this->buffer->Bind();
}

GLBufferLock::~GLBufferLock()
{
    buffer->Unbind();
}
