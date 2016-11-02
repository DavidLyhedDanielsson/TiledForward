#ifndef GLUNIFORM_H__
#define GLUNIFORM_H__

#include <cstring>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "glEnums.h"

template<typename T>
struct GLUniform;
template<typename T>
struct GLUniformArray;

// Needed to allow GLUniform to be placed in vector
class GLUniformBase
{
public:
    virtual void UploadData() = 0;
    virtual bool VerifyType(GLEnums::UNIFORM_TYPE type) = 0;

    GLint GetLocation() const { return location; }
    void SetLocation(GLint location) { this->location = location; }

    // This solution feels smart but I don't know
    // Faster in my benchmark than a switch statement though
    template<typename T>
    void operator=(T other)
    {
#ifndef NDEBUG
        GLUniform<T>* thisAsDerived = dynamic_cast<GLUniform<T>*>(this);

        if(thisAsDerived != nullptr)
            thisAsDerived->SetData(other);
#else // NDEBUG
        SetData(&other);
#endif // NDEBUG
    }

protected:
    GLint location;

#ifdef NDEBUG
    // In release mode type-safety is ignored in favour of performance
    virtual void SetData(void* data) = 0;
#endif // NDEBUG
};

template<typename T>
struct GLUniform
        : public GLUniformBase
{
public:
    GLUniform(T data)
    {
        this->data = data;
    }

#ifndef NDEBUG
    virtual void SetData(T data)
    {
        this->data = data;
    }
#endif // NDEBUG

    // TODO: Did this not work sometimes when optimizations are on?
    void UploadData()
    {
        throw std::runtime_error("Unknown data type, update glUniform.cpp");
    }
    bool VerifyType(GLEnums::UNIFORM_TYPE type)
    {
        throw std::runtime_error("Unknown GLEnums::UNIFORM_TYPE type, update glUniform.cpp");
    }

    const T GetData() const { return data; }

protected:
    T data;

#ifdef NDEBUG
    void SetData(void* data)
    {
        std::memcpy(&this->data, data, sizeof(T));
    }
#endif // NDEBUG
};

template<typename T>
struct GLUniformArray
        : public GLUniform<T>
{
public:
    GLUniformArray(T data, GLsizei count)
            : GLUniform<T>(new typename std::remove_pointer<T>::type[count])
              , count(count)
    {
        std::memcpy(this->data, data, sizeof(typename std::remove_pointer<T>::type) * count);
    }
    virtual ~GLUniformArray()
    {
        delete[] this->data;
    }

    // TODO: Rule of four½

    // TODO: Did this not work sometimes when optimizations are on?
    void UploadData()
    {
        throw std::runtime_error("Unknown data type, update glUniform.cpp");
    }

    GLsizei GetCount() const { return count; }

#ifndef NDEBUG
    void SetData(T data)
    {
        std::memcpy(this->data, data, sizeof(typename std::remove_pointer<T>::type) * count);
    }
#endif // NDEBUG

protected:
    GLsizei count;

#ifdef NDEBUG
    void SetData(void* data)
    {
        std::memcpy(this->data, *static_cast<T*>(data), sizeof(typename std::remove_pointer<T>::type) * count);
    }
#endif // NDEBUG
};

#endif // GLUNIFORM_H__
