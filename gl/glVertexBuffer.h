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
    void Init(GLEnums::BUFFER_USAGE usage, void* initialData, size_t elementCount)
    {
#ifndef GL_VERTEX_NO_WARNING
        Logger::LogLine(LOG_TYPE::WARNING
                        , "GLVertexBuffer::Init which takes a void* is used. Prefer using the std::vector version instead. To squelch this warning define GL_VERTEX_NO_WARNING");
#endif // GL_VERTEX_NO_WARNING

        this->stride = (GLsizei)GetByteSize<T...>();
        AddOffsets<T...>(this->offsets);

        GLBufferBase::Init<T...>(GLEnums::BUFFER_TYPE::VERTEX, usage, initialData, elementCount);
    }

    /**
     * Safely initializes this buffer with data
     *
     * @example
     * Initializing with a anonymous array. Input data is vertex position and vertex color, data type is float
     * vertexBuffer.Init<float, glm::vec3, glm::vec3>(GLEnums::BUFFER_USAGE::STATIC,
                {
                        -0.5f , -0.5f, 0.0f, 1.0f, 0.0f, 0.0f
                        , 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f
                        , -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f
                        , 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f
                }, false);
     * @tparam VectorType data type of data in vector
     * @tparam T vertex layout, may be multiple types, see example
     * @param usage
     * @param initialData
     * @param memcpyData Should the data be copied, or can a pointer to the first element inside \p initialData be used?
     * @return Whether or not initialization succeeded
     */
    template<typename VectorType, typename... T>
    void Init(GLEnums::BUFFER_USAGE usage, const std::vector<VectorType>& initialData)
    {
        // TODO: Make sure initialData.size matches stride

        this->stride = (GLsizei)GetByteSize<T...>();
        AddOffsets<T...>(this->offsets);

        GLBufferBase::Init<T...>(GLEnums::BUFFER_TYPE::VERTEX, usage, const_cast<VectorType*>(&initialData[0]), initialData.size());
    }

    GLsizei GetStride() const;
    const std::vector<size_t>& GetOffsets() const;

protected:
    template<typename First>
    void AddOffsets(std::vector<size_t>& offsets)
    {
        //Only one element => no offsets needed?
        /*if(offsets.size() == 0)
            offsets.push_back(0);

        offsets.push_back(offsets.back() + sizeof(First));*/
    }

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
