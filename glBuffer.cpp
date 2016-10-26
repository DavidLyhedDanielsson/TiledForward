#include "glBuffer.h"

#include <cstring>

GLBuffer::GLBuffer()
        : buffer(0)
        , bindType(0)
        , usage(0)
        , size(0)
        , data(nullptr)
        , memcpyData(false)
{ }

GLBuffer::~GLBuffer()
{
    if(memcpyData)
        free(data);
}

bool GLBuffer::Init(GLEnums::BUFFER_TYPE bufferType, GLEnums::BUFFER_USAGE usage, void* data, size_t dataSize, bool memcpyData)
{
    InitInternal(bufferType, usage, data, dataSize, memcpyData);
}

bool GLBuffer::InitInternal(GLEnums::BUFFER_TYPE bufferType, GLEnums::BUFFER_USAGE usage, void* data, size_t dataSize, bool memcpyData)
{
    this->bindType = (GLuint)bufferType;
    this->usage = (GLuint)usage;
    this->size = dataSize;
    this->memcpyData = memcpyData;

    if(memcpyData)
    {
        this->data = malloc(dataSize);
        std::memcpy(this->data, data, dataSize);
    }
    else
        this->data = data;

    glGenBuffers(1, &buffer);
    GLBufferLock lock(this);
    glBufferData(bindType, dataSize, this->data, this->usage);

    return false;
}

void GLBuffer::Bind()
{
    glBindBuffer(bindType, buffer);
}

void GLBuffer::Unbind()
{
    glBindBuffer(bindType, 0);
}

uint32_t GLBuffer::roundToNextPower(uint32_t value) const
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

uint64_t GLBuffer::roundToNextPower(uint64_t value) const
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

GLBufferLock::GLBufferLock(GLBuffer* buffer)
    : buffer(buffer)
{
    buffer->Bind();
}

GLBufferLock::~GLBufferLock()
{
    buffer->Unbind();
}
