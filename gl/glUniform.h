#ifndef GLUNIFORM_H__
#define GLUNIFORM_H__

#include <cstring>

#include <GL/gl3w.h>
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
    GLUniformBase()
            : location(-1)
    { }

    virtual void UploadData() = 0;
    virtual bool VerifyType(GLEnums::UNIFORM_TYPE type) = 0;

    GLint GetLocation() const { return location; }
    void SetLocation(GLint location) { this->location = location; }

    // This solution feels smart but I don't know
    // Faster in my benchmark than a switch statement though
    template<typename T>
    void operator=(const T other)
    {
#ifndef NDEBUG
        GLUniform<T>* thisAsDerived = dynamic_cast<GLUniform<T>*>(this);

        if(thisAsDerived != nullptr)
            thisAsDerived->SetData(other);
        else
            throw std::runtime_error("Couldn't cast type to T");
#else // NDEBUG
        SetData(&other);
#endif // NDEBUG
    }

protected:
    GLint location;

#ifdef NDEBUG
    // In release mode type-safety is ignored in favour of performance
    virtual void SetData(const void* data) = 0;
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
    void SetData(const void* data)
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
        throw std::runtime_error("Unknown data type, update glUniform.h");
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
    void SetData(const void* data)
    {
        std::memcpy(this->data, *static_cast<const T*>(data), sizeof(typename std::remove_pointer<T>::type) * count);
    }
#endif // NDEBUG
};

template<>
inline void GLUniform<GLfloat>::UploadData() { glUniform1f(location, data); }

template<>
inline void GLUniform<glm::vec2>::UploadData() { glUniform2f(location, data.x, data.y); }

template<>
inline void GLUniform<glm::vec3>::UploadData() { glUniform3f(location, data.x, data.y, data.z); }

template<>
inline void GLUniform<glm::vec4>::UploadData() { glUniform4f(location, data.x, data.y, data.z, data.w); }

template<>
inline void GLUniform<GLint>::UploadData() { glUniform1i(location, data); }

template<>
inline void GLUniform<glm::ivec2>::UploadData() { glUniform2i(location, data.x, data.y); }

template<>
inline void GLUniform<glm::ivec3>::UploadData() { glUniform3i(location, data.x, data.y, data.z); }

template<>
inline void GLUniform<glm::ivec4>::UploadData() { glUniform4i(location, data.x, data.y, data.z, data.w); }

template<>
inline void GLUniform<GLuint>::UploadData() { glUniform1ui(location, data); }

template<>
inline void GLUniform<glm::uvec2>::UploadData() { glUniform2ui(location, data.x, data.y); }

template<>
inline void GLUniform<glm::uvec3>::UploadData() { glUniform3ui(location, data.x, data.y, data.z); }

template<>
inline void GLUniform<glm::uvec4>::UploadData() { glUniform4ui(location, data.x, data.y, data.z, data.w); }

template<>
inline void GLUniform<glm::mat2x2>::UploadData() { glUniformMatrix2fv(location, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }

template<>
inline void GLUniform<glm::mat3x3>::UploadData() { glUniformMatrix3fv(location, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }

template<>
inline void GLUniform<glm::mat4x4>::UploadData() { glUniformMatrix4fv(location, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }

template<>
inline void GLUniform<glm::mat2x3>::UploadData() { glUniformMatrix2x3fv(location, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }

template<>
inline void GLUniform<glm::mat3x2>::UploadData() { glUniformMatrix3x2fv(location, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }

template<>
inline void GLUniform<glm::mat2x4>::UploadData() { glUniformMatrix2x4fv(location, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }

template<>
inline void GLUniform<glm::mat4x2>::UploadData() { glUniformMatrix4x2fv(location, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }

template<>
inline void GLUniform<glm::mat3x4>::UploadData() { glUniformMatrix3x4fv(location, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }

template<>
inline void GLUniform<glm::mat4x3>::UploadData() { glUniformMatrix4x3fv(location, 1, GL_FALSE, reinterpret_cast<GLfloat*>(&data)); }

template<>
inline void GLUniformArray<GLfloat*>::UploadData() { glUniform1fv(location, count, data); }

template<>
inline void GLUniformArray<glm::vec2*>::UploadData() { glUniform2fv(location, count, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::vec3*>::UploadData() { glUniform3fv(location, count, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::vec4*>::UploadData() { glUniform4fv(location, count, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<GLint*>::UploadData() { glUniform1iv(location, count, data); }

template<>
inline void GLUniformArray<glm::ivec2*>::UploadData() { glUniform2iv(location, count, reinterpret_cast<GLint*>(data)); }

template<>
inline void GLUniformArray<glm::ivec3*>::UploadData() { glUniform3iv(location, count, reinterpret_cast<GLint*>(data)); }

template<>
inline void GLUniformArray<glm::ivec4*>::UploadData() { glUniform4iv(location, count, reinterpret_cast<GLint*>(data)); }

template<>
inline void GLUniformArray<GLuint*>::UploadData() { glUniform1uiv(location, count, data); }

template<>
inline void GLUniformArray<glm::uvec2*>::UploadData() { glUniform2uiv(location, count, reinterpret_cast<GLuint*>(data)); }

template<>
inline void GLUniformArray<glm::uvec3*>::UploadData() { glUniform3uiv(location, count, reinterpret_cast<GLuint*>(data)); }

template<>
inline void GLUniformArray<glm::uvec4*>::UploadData() { glUniform4uiv(location, count, reinterpret_cast<GLuint*>(data)); }

template<>
inline void GLUniformArray<glm::mat2x2*>::UploadData() { glUniformMatrix2fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::mat3x3*>::UploadData() { glUniformMatrix3fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::mat4x4*>::UploadData() { glUniformMatrix4fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::mat2x3*>::UploadData() { glUniformMatrix2x3fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::mat3x2*>::UploadData() { glUniformMatrix3x2fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::mat2x4*>::UploadData() { glUniformMatrix2x4fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::mat4x2*>::UploadData() { glUniformMatrix4x2fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::mat3x4*>::UploadData() { glUniformMatrix3x4fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
inline void GLUniformArray<glm::mat4x3*>::UploadData() { glUniformMatrix4x3fv(location, count, GL_FALSE, reinterpret_cast<GLfloat*>(data)); }

template<>
inline bool GLUniform<GLfloat>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT; }

template<>
inline bool GLUniform<glm::vec2>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC2; }

template<>
inline bool GLUniform<glm::vec3>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC3; }

template<>
inline bool GLUniform<glm::vec4>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC4; }

template<>
inline bool GLUniform<GLint>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT; }

template<>
inline bool GLUniform<glm::ivec2>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC2; }

template<>
inline bool GLUniform<glm::ivec3>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC3; }

template<>
inline bool GLUniform<glm::ivec4>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC4; }

template<>
inline bool GLUniform<GLuint>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT; }

template<>
inline bool GLUniform<glm::uvec2>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC2; }

template<>
inline bool GLUniform<glm::uvec3>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC3; }

template<>
inline bool GLUniform<glm::uvec4>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC4; }

template<>
inline bool GLUniform<glm::mat2x2>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT2; }

template<>
inline bool GLUniform<glm::mat3x3>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT3; }

template<>
inline bool GLUniform<glm::mat4x4>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT4; }

template<>
inline bool GLUniform<glm::mat2x3>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT2X3; }

template<>
inline bool GLUniform<glm::mat3x2>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT3X2; }

template<>
inline bool GLUniform<glm::mat2x4>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT2X4; }

template<>
inline bool GLUniform<glm::mat4x2>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT4X2; }

template<>
inline bool GLUniform<glm::mat3x4>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT3X4; }

template<>
inline bool GLUniform<glm::mat4x3>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT4X3; }

template<>
inline bool GLUniform<GLfloat*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT; }

template<>
inline bool GLUniform<glm::vec2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC2; }

template<>
inline bool GLUniform<glm::vec3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC3; }

template<>
inline bool GLUniform<glm::vec4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_VEC4; }

template<>
inline bool GLUniform<GLint*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT; }

template<>
inline bool GLUniform<glm::ivec2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC2; }

template<>
inline bool GLUniform<glm::ivec3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC3; }

template<>
inline bool GLUniform<glm::ivec4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC4; }

template<>
inline bool GLUniform<GLuint*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT; }

template<>
inline bool GLUniform<glm::uvec2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC2; }

template<>
inline bool GLUniform<glm::uvec3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC3; }

template<>
inline bool GLUniform<glm::uvec4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::INT_VEC4; }

template<>
inline bool GLUniform<glm::mat2x2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT2; }

template<>
inline bool GLUniform<glm::mat3x3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT3; }

template<>
inline bool GLUniform<glm::mat4x4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT4; }

template<>
inline bool GLUniform<glm::mat2x3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT2X3; }

template<>
inline bool GLUniform<glm::mat3x2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT3X2; }

template<>
inline bool GLUniform<glm::mat2x4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT2X4; }

template<>
inline bool GLUniform<glm::mat4x2*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT4X2; }

template<>
inline bool GLUniform<glm::mat3x4*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT3X4; }

template<>
inline bool GLUniform<glm::mat4x3*>::VerifyType(GLEnums::UNIFORM_TYPE type) { return type == GLEnums::UNIFORM_TYPE::FLOAT_MAT4X3; }

#endif // GLUNIFORM_H__
