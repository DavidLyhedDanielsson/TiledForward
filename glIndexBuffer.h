#ifndef GLINDEXBUFFER_H__
#define GLINDEXBUFFER_H__

#include "glBuffer.h"

class GLIndexBuffer
        : public GLBuffer
{
public:
    GLIndexBuffer();
    ~GLIndexBuffer();

    template<typename... T>
    bool Init(GLEnums::BUFFER_USAGE usage, void* initialData, GLsizei elementCount, bool memcpyData)
    {
        indiciesCount = elementCount;

        return GLBuffer::Init<T...>(GLEnums::BUFFER_TYPE::INDEX, usage, initialData, elementCount, memcpyData);
    }

    GLsizei GetIndiciesCount() const;

protected:
private:
    GLsizei indiciesCount;
};

#endif // GLINDEXBUFFER_H__
