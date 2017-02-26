#ifndef GLSHADERSTORAGEBUFFER_H__
#define GLSHADERSTORAGEBUFFER_H__

#include "glBufferBase.h"

class GLShaderStorageBuffer
{
    friend class GLDrawBinds;
public:
    GLShaderStorageBuffer(const std::string& name, GLuint shaderProgram, GLuint blockIndex, GLuint bindingPoint);
    ~GLShaderStorageBuffer();

    bool Init();
    void Bind();
    void Unbind();

    void Share(GLShaderStorageBuffer* other);
    void UpdateData(const size_t offset, void* data, int dataSize);

protected:
private:
    GLuint shaderProgram;
    GLuint bufferIndex;
    GLuint blockIndex;
    GLuint bindingPoint;
    std::string name;

    GLint size;

    void SetData(const void* data, size_t dataSize);
};

#endif // GLSHADERSTORAGEBUFFER_H__
