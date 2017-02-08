#ifndef GLINPUTLAYOUT_H__
#define GLINPUTLAYOUT_H__

#include <vector>
#include <string>

#include <GL/glew.h>

#include "glEnums.h"
#include "logger.h"
#include "glHelpers.h"

struct InputLayout
{
    InputLayout()
            : size(-1)
              , type(GLEnums::DATA_TYPE::FLOAT)
              , normalized(GL_FALSE)
              , stride(-1)
              , offset(-1)
              , vertexAttribDivisor(-1)
    {}

    InputLayout(GLint size
                , GLEnums::DATA_TYPE type
                , GLboolean normalized
                , GLsizei stride
                , size_t offset
                , GLint vertexAttribDivisor)
            : size(size)
              , type(type)
              , normalized(normalized)
              , stride(stride)
              , offset(offset)
              , vertexAttribDivisor(vertexAttribDivisor)
    {}

    GLint size;
    GLEnums::DATA_TYPE type;
    GLboolean normalized;
    GLsizei stride;
    size_t offset; // Cast to pointer later, so size_t should be used
    GLint vertexAttribDivisor;
};

struct GLInputLayout
{
    friend class GLDrawBinds;

    GLInputLayout();
    ~GLInputLayout();

    template<typename T>
    void SetInputLayout(int startLocation = 0)
    {
        AppendInputLayout<T>();

        GLsizei stride = 0;
        for(auto& pair : namedInputLayouts)
        {
            pair.second.offset = (size_t)stride;
            stride += pair.second.size * GLHelpers::Sizeof(pair.second.type);
        }

        for(int i = 0; i < namedInputLayouts.size(); ++i)
        {
            namedInputLayouts[i].first = std::to_string(startLocation + i);
            namedInputLayouts[i].second.stride = stride;
        }
    }

    //void AddInputLayout(GLint location, GLEnums::UNIFORM_TYPE type, GLboolean normalized, GLsizei stride, size_t offset);
    template<typename T, typename Second, typename... Rest>
    void SetInputLayout(int startLocation = 0) // TODO: Add string versions and explicit location versions
    {
        AppendInputLayout<T>();

        SetInputLayout<Second, Rest...>(startLocation);
    }

    void AddInputLayout(GLint location
                            , GLint size
                            , GLEnums::DATA_TYPE type
                            , GLboolean normalized
                            , GLsizei stride
                            , size_t offset
                            , GLint vertexDivisor);
    void AddInputLayout(const std::string& name
                            , GLint size
                            , GLEnums::DATA_TYPE type
                            , GLboolean normalized
                            , GLsizei stride
                            , size_t offset
                            , GLint vertexDivisor);

    template<typename... Rest>
    void SetDefaultInputLayout(const std::string& name, Rest... rest)
    {
        AddDefaultInputLayout(name);

        SetDefaultInputLayout(rest...);
    }

    void AddDefaultInputLayout(GLint location);
    void AddDefaultInputLayout(const std::string& name);

    void SetVertexAttribDivisor(int location, int divisor);

private:
    // Variadic template termination

    void SetDefaultInputLayout()
    { }

    template<typename T>
    void AppendInputLayout()
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Unknown input layout type");
    }

    std::vector<std::pair<std::string, InputLayout>> namedInputLayouts;
    size_t vertexBuffer;
};

template<>
inline void GLInputLayout::AppendInputLayout<int>()
{
    AddInputLayout((int)namedInputLayouts.size(), 1, GLEnums::DATA_TYPE::INT, GL_FALSE, -1, -1, 0);
}

template<>
inline void GLInputLayout::AppendInputLayout<float>()
{
    AddInputLayout((int)namedInputLayouts.size(), 1, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1, 0);
}

template<>
inline void GLInputLayout::AppendInputLayout<glm::vec2>()
{
    AddInputLayout((int)namedInputLayouts.size(), 2, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1, 0);
}

template<>
inline void GLInputLayout::AppendInputLayout<glm::vec3>()
{
    AddInputLayout((int)namedInputLayouts.size(), 3, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1, 0);
}

template<>
inline void GLInputLayout::AppendInputLayout<glm::vec4>()
{
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1, 0);
}

template<>
inline void GLInputLayout::AppendInputLayout<glm::mat4x4>()
{
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1, 0);
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1, 0);
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1, 0);
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1, 0);
}

#endif // GLINPUTLAYOUT_H__
