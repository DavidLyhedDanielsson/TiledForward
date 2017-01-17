#ifndef GLBUFFERBASE_H__
#define GLBUFFERBASE_H__

#include "glEnums.h"

#include "logger.h"

#include <vector>

class GLBufferBase
{
public:
    GLBufferBase();
    virtual ~GLBufferBase();

    GLBufferBase(const GLBufferBase& other) = delete;
    GLBufferBase(GLBufferBase&& other) = delete;
    GLBufferBase& operator=(const GLBufferBase& rhs) = delete;
    GLBufferBase& operator=(GLBufferBase&& rhs) = delete;

    // None of these should ever modify the data
    bool Init(GLEnums::BUFFER_TYPE bufferType, GLEnums::BUFFER_USAGE usage, void* data, size_t dataSize);
    template<typename... T>
    bool Init(GLEnums::BUFFER_TYPE bufferType, GLEnums::BUFFER_USAGE usage, void* initialData, size_t elementCount)
    {
        size_t byteSize = GetByteSize<T...>();
        return InitInternal(bufferType, usage, initialData, byteSize * elementCount);
    }

    void Bind();
    void Unbind();

    /**
     * Returns the data of this buffer formatted as a string
     *
     * @example Assuming the inner format is float, float, int, int:
     * buffer.GetData<float, float, int, int>();
     *
     * @return
     */
    template<typename First, typename... Rest>
    std::string GetData() const
    {
        std::stringstream sstream;

        size_t byteSize = GetByteSize<First, Rest...>();

        if(size % byteSize != 0)
        {
            //TODO: Better message
            Logger::LogLine(LOG_TYPE::WARNING, "size isn't a multiple of byteSize, check template parameters");
            sstream << "(size isn't a multiple of byteSize)\n";
        }

        glBindBuffer(bindType, buffer);
        void* data = glMapBuffer(bindType, GL_READ_ONLY);

        for(int i = 0, end = size / byteSize; i < end; ++i)
        {
            GetDataRec<First, Rest...>(sstream, static_cast<char*>(data) + byteSize * i);
            sstream << std::endl;
        }

        glUnmapBuffer(bindType);
        glBindBuffer(bindType, NULL);

        return sstream.str();
    }

    virtual void Update(const void* data, size_t size) = 0;

    bool IsBound() const;

protected:
    GLuint buffer;
    GLuint bindType;
    GLuint usage;

    size_t size;

    uint32_t roundToNextPower(uint32_t value) const;
    uint64_t roundToNextPower(uint64_t value) const;

    /**@{*/
    /**
     * Returns the byte size of the given template arguments
     *
     * @example GetByteSize<float, float, int>() returns 12
     * @param size
     * @return
     */
    template<typename T>
    size_t GetByteSize() const
    {
        return sizeof(T);
    }

    template<typename First, typename Second, typename... Rest>
    size_t GetByteSize() const
    {
        return GetByteSize<First>() + GetByteSize<Second, Rest...>();
    };
    /**@}*/
private:
    bool isBound;

    bool InitInternal(GLEnums::BUFFER_TYPE bufferType, GLEnums::BUFFER_USAGE usage, void* data, size_t dataSize);

    /**@{*/
    /**
     * @see GetData
     */
    template<typename Type>
    void GetDataRec(std::stringstream& sstream, void* data) const
    {
        sstream << *reinterpret_cast<Type*>(data) << ", ";
    }

    template<typename First, typename Second, typename... Rest>
    void GetDataRec(std::stringstream& sstream, void* data) const
    {
        GetDataRec<First>(sstream, data);
        GetDataRec<Second, Rest...>(sstream, static_cast<char*>(data) + sizeof(First));
    }
    /**@}*/
};

class GLBufferLock
{
public:
    GLBufferLock(GLBufferBase* buffer);
    GLBufferLock(GLBufferBase& buffer);
    ~GLBufferLock();

private:
    GLBufferBase* buffer;
};

#endif //GLBUFFERBASE_H__
