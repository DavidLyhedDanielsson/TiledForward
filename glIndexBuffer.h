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
    bool Init(GLEnums::BUFFER_USAGE usage, void* initialData, GLsizei elementCount)
    {
#ifndef GL_INDEX_NO_WARNING
        Logger::LogLine(LOG_TYPE::WARNING
                        , "GLIndexBuffer::Init which takes a void* is used. Prefer using the std::vector version instead. To squelch this warning define GL_INDEX_NO_WARNING");
#endif // GL_INDEX_NO_WARNING

        indexCount = elementCount;

        return GLBufferBase::Init<T...>(GLEnums::BUFFER_TYPE::INDEX, usage, initialData, elementCount);
    }

    /**
     * Safely initializes this buffer with data
     *
     * @example
     * indexBuffer.Init(GLEnums::BUFFER_USAGE::STATIC, { 0, 2, 1, 3, 1, 2 }, false);
     *
     * @param usage
     * @param indicies
     * @param memcpyData Should the data be copied, or can a pointer to the first element inside \p indicies be used?
     * @return Whether or not initialization succeeded
     */
    bool Init(GLEnums::BUFFER_USAGE usage, const std::vector<GLint>& indicies)
    {
        indexCount = (GLsizei)indicies.size();

        // GLBufferBase::Init never modifies the data
        return GLBufferBase::Init<GLint>(GLEnums::BUFFER_TYPE::INDEX, usage, const_cast<GLint*>(&indicies[0]), indicies.size());
    }

    void Update(const std::vector<GLuint>& indicies);

    GLsizei GetIndiciesCount() const;

protected:
private:
    GLsizei indexCount;
};

#endif // GLINDEXBUFFER_H__
