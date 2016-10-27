#ifndef VERTEXBUFFER_H__
#define VERTEXBUFFER_H__

#include "glBuffer.h"

class GLVertexBuffer
    : public GLBuffer
{
public:
    GLVertexBuffer();
    ~GLVertexBuffer();

    template<typename... T>
    bool Init(GLEnums::BUFFER_USAGE usage, void* initialData, size_t elementCount, bool memcpyData)
    {
        this->stride = (GLsizei)GetByteSize<T...>();
        AddOffsets<T...>(this->offsets);

        return GLBuffer::Init<T...>(GLEnums::BUFFER_TYPE::VERTEX, usage, initialData, elementCount, memcpyData);
    }

    GLsizei GetStride() const;
    const std::vector<size_t>& GetOffsets() const;

protected:
    template<typename First>
    void AddOffsets(std::vector<size_t>& offsets)
    { }

    template<typename First, typename Second, typename... Rest>
    void AddOffsets(std::vector<size_t>& offsets)
    {
        if(offsets.size() == 0)
            offsets.push_back(0);

        offsets.push_back(offsets.back() + sizeof(First));

        AddOffsets<Second, Rest...>(offsets);
    }

private:
    GLsizei stride;
    std::vector<size_t> offsets;
};

#endif // VERTEXBUFFER_H__
