#ifndef VERTEXBUFFER_H__
#define VERTEXBUFFER_H__

#include "glBuffer.h"

class VertexBuffer
    : public GLBuffer
{
public:
    VertexBuffer();
    ~VertexBuffer();

    bool Init(GLEnums::BUFFER_USAGE usage, void* data, size_t dataSize, bool memcpyData);
    template<typename T>
    bool Init(GLEnums::BUFFER_USAGE usage, const std::vector<T>& initialData, bool memcpyData)
    {
        return GLBuffer::Init(GLEnums::BUFFER_TYPE::VERTEX, usage, &initialData[0], initialData.size(), memcpyData);
    }
    template<typename... T>
    bool Init(GLEnums::BUFFER_USAGE usage, void* initialData, size_t elementCount, bool memcpyData)
    {
        return GLBuffer::Init<T...>(GLEnums::BUFFER_TYPE::VERTEX, usage, initialData, elementCount, memcpyData);
    }
};

#endif // VERTEXBUFFER_H__
