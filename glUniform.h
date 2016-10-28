#ifndef GLUNIFORM_H__
#define GLUNIFORM_H__

#include <cstring>

#include <GL/glew.h>
#include <glm/glm.hpp>

template<typename T>
struct GLUniform;
template<typename T>
struct GLUniformArray;

// Needed to allow GLUniform to be placed in vector
class GLUniformBase
{
public:
    virtual void UploadData() = 0;

    GLint GetLocation() const { return location; }
    void SetLocation(GLint location) { this->location = location; }

    // This solution feels smart but I don't know
    // Faster in my benchmark than a switch statement though
    template<typename T>
    void operator=(T other)
    {
        // TODO: Avoid two dynamic_cast somehow?
        GLUniform<T>* thisAsDerived = dynamic_cast<GLUniform<T>*>(this);

        if(thisAsDerived != nullptr)
            thisAsDerived->SetData(other);
        else
        {
            GLUniformArray<T>* thisAsDerivedArray = dynamic_cast<GLUniformArray<T>*>(this);

            if(thisAsDerivedArray != nullptr)
                thisAsDerivedArray->SetData(other);
            else
                throw std::runtime_error("Trying to set uniform with wrong data type"); //TODO: Do something better than throwing?
        }
    }

protected:
    GLint location;
};

template<typename T>
struct GLUniform
    : public GLUniformBase
{
public:
    GLUniform(T data)
            : data(data)
    { }

    GLUniform(T* data)
            : data(*data)
    { }

    void UploadData();

    T GetData() const { return data; }
    void SetData(T data) { this->data = data; }

protected:
    T data;
};

template<typename T>
struct GLUniformArray
        : public GLUniformBase
{
public:
    GLUniformArray(T data, GLsizei count)
            : data(new typename std::remove_pointer<T>::type[count])
              , count(count)
    {
        std::memcpy(this->data, data, sizeof(typename std::remove_pointer<T>::type) * count);
    }
    virtual ~GLUniformArray()
    {
        delete[] data;
    }

    // TODO: Rule of four½

    void UploadData();

    GLsizei GetCount() const { return count; }
    T GetData() const { return data; }
    void SetData(T data)
    {
        std::memcpy(this->data, data, sizeof(T) * count);
    }

protected:
    GLsizei count;
    T data;
};

#endif // GLUNIFORM_H__
