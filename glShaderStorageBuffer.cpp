#include "glShaderStorageBuffer.h"

#include <cstring>

GLShaderStorageBuffer::GLShaderStorageBuffer(const std::string& name, GLuint shaderProgram, GLuint blockIndex, GLuint bindingPoint)
        : name(name)
          , size(0)
          , shaderProgram(shaderProgram)
          , blockIndex(blockIndex)
          , bindingPoint(bindingPoint)
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

bool GLShaderStorageBuffer::Init()
{
    glGenBuffers(1, &bufferIndex);

    glShaderStorageBlockBinding(shaderProgram, blockIndex, bindingPoint);

    return true;
}

void GLShaderStorageBuffer::SetData(const void* data, size_t dataSize)
{
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