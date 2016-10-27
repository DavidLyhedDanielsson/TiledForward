#ifndef GLUNIFORM_H__
#define GLUNIFORM_H__

#include <GL/glew.h>
#include <glm/glm.hpp>

template<typename T>
struct GLUniform;

//Needed to allow GLUniform to be placed in vector
class GLUniformBase
{
public:
    virtual void UploadData() = 0;

    GLint GetLocation() const { return location; }
    void SetLocation(GLint location) { this->location = location; }

    //This solution feels smart but I don't know
    template<typename T>
    void operator=(T other)
    {
        GLUniform<T>* thisAsDerived = dynamic_cast<GLUniform<T>*>(this);

        if(thisAsDerived != nullptr)
            thisAsDerived->SetData(other);
        else
            throw std::runtime_error("Trying to set uniform with wrong data type"); //TODO: Do something better than throwing?
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
    GLint location;
    T data;
};

template<typename T>
struct GLUniformArray
        : public GLUniformBase
{
public:
    GLUniformArray()
            : dataOwned(false)
    {}
    virtual ~GLUniformArray()
    {
        if(dataOwned)
            delete[] data;
    }

    GLUniformArray(const GLUniformArray& other)
            : location(other.location)
              , dataOwned(other.dataOwned)
              , data(nullptr) //Make sure this is initialized to nullptr so that delete[] data won't do anything
    {
        SetData(other.data, other.count, false);
    }

    GLUniformArray(GLUniformArray&& other)
    {
        std::swap(*this, other);
    }

    void operator=(GLUniformArray rhs)
    {
        location = rhs.location;
        count = rhs.count;
        data = rhs.data;
        dataOwned = rhs.dataOwned;
    }

    void UploadData();

    GLsizei GetCount() const { return count; }
    T GetData() const { return data; }
    void SetData(T* data, GLsizei count, bool memcpyData)
    {
        if(dataOwned)
            delete[] data;

        if(memcpyData)
            data = new T[count];

        dataOwned = false;

        this->data = data;
        this->count = count;
    }

protected:
    GLint location;
    GLsizei count;
    T* data;

    bool dataOwned;
};

//template<typename T>
//void GLUniform<T>::UploadData() {  }
//
//template<>
//void GLUniform<GLfloat>::UploadData() { glUniform1f(location, data); }
//
//struct GLUniform2f : public GLUniform<glm::vec2>
//{
//    void UploadData() { glUniform2f(location, data.x, data.y); }
//};
//
//struct GLUniform3f : public GLUniform<glm::vec3>
//{
//    void UploadData() { glUniform3f(location, data.x, data.y, data.z); }
//};
//
//struct GLUniform4f : public GLUniform<glm::vec4>
//{
//    void UploadData() { glUniform4f(location, data.x, data.y, data.z, data.w); }
//};
//
//struct GLUniform1i : public GLUniform<GLint>
//{
//    void UploadData() { glUniform1i(location, data); }
//};
//
//struct GLUniform2i : public GLUniform<glm::ivec2>
//{
//    void UploadData() { glUniform2i(location, data.x, data.y); }
//};
//
//struct GLUniform3i : public GLUniform<glm::ivec3>
//{
//    void UploadData() { glUniform3i(location, data.x, data.y, data.z); }
//};
//
//struct GLUniform4i : public GLUniform<glm::ivec4>
//{
//    void UploadData() { glUniform4i(location, data.x, data.y, data.z, data.w); }
//};
//
//struct GLUniform1ui : public GLUniform<GLuint>
//{
//    void UploadData() { glUniform1ui(location, data); }
//};
//
//struct GLUniform2ui : public GLUniform<glm::uvec2>
//{
//    void UploadData() { glUniform2ui(location, data.x, data.y); }
//};
//
//struct GLUniform3ui : public GLUniform<glm::uvec3>
//{
//    void UploadData() { glUniform3ui(location, data.x, data.y, data.z); }
//};
//
//struct GLUniform4ui : public GLUniform<glm::uvec4>
//{
//    void UploadData() { glUniform4ui(location, data.x, data.y, data.z, data.w); }
//};
//
//struct GLUniform1fv : public GLUniformArray<GLfloat>
//{
//    void UploadData() { glUniform1fv(location, count, data); }
//};
//
//struct GLUniform2fv : public GLUniformArray<glm::vec2>
//{
//    void UploadData() { glUniform2fv(location, count, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniform3fv : public GLUniformArray<glm::vec3>
//{
//    void UploadData() { glUniform3fv(location, count, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniform4fv : public GLUniformArray<glm::vec4>
//{
//    void UploadData() { glUniform4fv(location, count, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniform1iv : public GLUniformArray<GLint>
//{
//    void UploadData() { glUniform1iv(location, count, data); }
//};
//
//struct GLUniform2iv : public GLUniformArray<glm::ivec2>
//{
//    void UploadData() { glUniform2iv(location, count, reinterpret_cast<GLint*>(&data)); }
//};
//
//struct GLUniform3iv : public GLUniformArray<glm::ivec3>
//{
//    void UploadData() { glUniform3iv(location, count, reinterpret_cast<GLint*>(&data)); }
//};
//
//struct GLUniform4iv : public GLUniformArray<glm::ivec4>
//{
//    void UploadData() { glUniform4iv(location, count, reinterpret_cast<GLint*>(&data)); }
//};
//
//struct GLUniform1uiv : public GLUniformArray<GLuint>
//{
//    void UploadData() { glUniform1uiv(location, count, data); }
//};
//
//struct GLUniform2uiv : public GLUniformArray<glm::uvec2>
//{
//    void UploadData() { glUniform2uiv(location, count, reinterpret_cast<GLuint*>(&data)); }
//};
//
//struct GLUniform3uiv : public GLUniformArray<glm::uvec3>
//{
//    void UploadData() { glUniform3uiv(location, count, reinterpret_cast<GLuint*>(&data)); }
//};
//
//struct GLUniform4uiv : public GLUniformArray<glm::uvec4>
//{
//    void UploadData() { glUniform4uiv(location, count, reinterpret_cast<GLuint*>(&data)); }
//};
//
//struct GLUniformMatrix2fv : public GLUniformArray<glm::mat2x2>
//{
//    void UploadData() { glUniformMatrix2fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniformMatrix3fv : public GLUniformArray<glm::mat3x3>
//{
//    void UploadData() { glUniformMatrix3fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniformMatrix4fv : public GLUniformArray<glm::mat4x4>
//{
//    void UploadData() { glUniformMatrix4fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniformMatrix2x3fv : public GLUniformArray<glm::mat2x3>
//{
//    void UploadData() { glUniformMatrix2x3fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniformMatrix3x2fv : public GLUniformArray<glm::mat3x2>
//{
//    void UploadData() { glUniformMatrix3x2fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniformMatrix2x4fv : public GLUniformArray<glm::mat2x4>
//{
//    void UploadData() { glUniformMatrix2x4fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniformMatrix4x2fv : public GLUniformArray<glm::mat4x2>
//{
//    void UploadData() { glUniformMatrix4x2fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniformMatrix3x4fv : public GLUniformArray<glm::mat3x4>
//{
//    void UploadData() { glUniformMatrix3x4fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }
//};
//
//struct GLUniformMatrix4x3fv : public GLUniformArray<glm::mat4x3>
//{
//    void UploadData() { glUniformMatrix4x3fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }
//};

#endif // GLUNIFORM_H__
