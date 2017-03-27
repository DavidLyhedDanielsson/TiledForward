#include "glShaderStorageBuffer.h"

#include <cstring>
#include <assert.h>

GLShaderStorageBuffer::GLShaderStorageBuffer(const std::string& name, GLuint shaderProgram, GLuint blockIndex, GLuint bindingPoint)
        : name(name)
          , size(0)
          , shaderProgram(shaderProgram)
          , blockIndex(blockIndex)
          , bindingPoint(bindingPoint)
          , deallocateOnShare(true)
{ }

GLShaderStorageBuffer::~GLShaderStorageBuffer()
{}

void GLShaderStorageBuffer::Bind()
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, bufferIndex);
}

void GLShaderStorageBuffer::Unbind()
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, 0);
}

bool GLShaderStorageBuffer::Init(bool bind)
{
    glGenBuffers(1, &bufferIndex);

    if(bind)
        glShaderStorageBlockBinding(shaderProgram, blockIndex, bindingPoint);

    return true;
}

void GLShaderStorageBuffer::SetData(const void* data, size_t dataSize)
{
    if(dataSize == 0)
    {
        Logger::LogLine(LOG_TYPE::WARNING, "Tryint to update buffer with dataSize of 0");
        return;
    }

    if(dataSize > this->size)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIndex);
        glBufferData(GL_SHADER_STORAGE_BUFFER, dataSize, data, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        this->size = dataSize;
    }
    else
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIndex);
        void* mappedData = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY | GL_MAP_INVALIDATE_BUFFER_BIT);
        std::memcpy(mappedData, data, dataSize);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
}

void GLShaderStorageBuffer::SetData(GLDynamicBuffer* buffer)
{
    if(buffer->GetTotalSize() != this->size)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIndex);
        glBufferData(GL_SHADER_STORAGE_BUFFER, buffer->GetTotalSize(), nullptr, GL_DYNAMIC_DRAW);

        this->size = buffer->GetTotalSize();
    }
    else
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIndex);

    void* mappedData = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY | GL_MAP_INVALIDATE_BUFFER_BIT);
    buffer->UploadData(mappedData);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GLShaderStorageBuffer::Share(GLShaderStorageBuffer* other)
{
    if(bufferIndex == other->bufferIndex)
        return;

    glDeleteBuffers(1, &bufferIndex);

    //this->bindingPoint = other->bindingPoint;
    this->bufferIndex = other->bufferIndex;
    this->size = other->size;

    glShaderStorageBlockBinding(shaderProgram, blockIndex, bindingPoint);
}

void GLShaderStorageBuffer::UpdateData(const size_t offset, void* data, int dataSize)
{
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIndex);
    void* mappedData = glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, dataSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    std::memcpy(mappedData, data, dataSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void GLShaderStorageBuffer::SetData(int value)
{
    void* data = malloc(size);
    std::memset(data, value, this->size);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIndex);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, this->size, data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    free(data);
}

std::unique_ptr<void, UniquePtrFree> GLShaderStorageBuffer::GetData() const
{
    void* data = malloc((size_t)this->size);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bufferIndex);
    void* mappedData = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    std::memcpy(data, mappedData, this->size);

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    return std::unique_ptr<void, UniquePtrFree>(data);
}

int GLShaderStorageBuffer::GetSize() const
{
    return size;
}

void GLShaderStorageBuffer::Replace(GLShaderStorageBuffer* other)
{
    assert(this->shaderProgram != -1
           && blockIndex != -1
           && bindingPoint != -1);

    if(deallocateOnShare)
        glDeleteBuffers(1, &bufferIndex);

    deallocateOnShare = false;

    this->bufferIndex = other->bufferIndex;
    this->size = other->size;

    glShaderStorageBlockBinding(shaderProgram, blockIndex, bindingPoint);
}
