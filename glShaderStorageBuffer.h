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

    bool Init();
    void Bind();
    void Unbind();

    void Share(GLShaderStorageBuffer* other);
    void UpdateData(const size_t offset, void* data, int dataSize);

    std::unique_ptr<void, UniquePtrFree> GetData() const;

protected:
private:
    GLuint shaderProgram;
    GLuint bufferIndex;
    GLuint blockIndex;
    GLuint bindingPoint;
    std::string name;

    GLint size;

    void SetData(const void* data, size_t dataSize);
    void SetData(GLDynamicBuffer* buffer);
};

#endif // GLSHADERSTORAGEBUFFER_H__
