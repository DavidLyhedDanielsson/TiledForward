#ifndef GLSHADERSTORAGEBUFFER_H__
#define GLSHADERSTORAGEBUFFER_H__

#include <memory>
#include "glBufferBase.h"
#include "glDynamicBuffer.h"

struct UniquePtrFree
{
    void operator()(void* pointer)
    {
        free(pointer);
    }
};

class GLShaderStorageBuffer
{
    friend class GLDrawBinds;
public:
    GLShaderStorageBuffer(const std::string& name, GLuint shaderProgram, GLuint blockIndex, GLuint bindingPoint);
    ~GLShaderStorageBuffer();

    bool Init(bool bind = true);
    void Bind();
    void Unbind();

    void Share(GLShaderStorageBuffer* other);
    void Replace(GLShaderStorageBuffer* other);
    void UpdateData(const size_t offset, void* data, int dataSize);
    void SetData(int value);
    void SetData(const void* data, size_t dataSize);

    std::unique_ptr<void, UniquePtrFree> GetData() const;

    int GetSize() const;

protected:
private:
    GLuint shaderProgram;
    GLuint bufferIndex;
    GLuint blockIndex;
    GLuint bindingPoint;
    std::string name;

    GLint size;

    bool deallocateOnShare;

    void SetData(GLDynamicBuffer* buffer);
};

#endif // GLSHADERSTORAGEBUFFER_H__
