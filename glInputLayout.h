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
    GLint size;
    GLEnums::DATA_TYPE type;
    GLboolean normalized;
    GLsizei stride;
    size_t offset; // Cast to pointer later, so size_t should be used
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

    void AddInputLayout(GLint location, GLint size, GLEnums::DATA_TYPE type, GLboolean normalized, GLsizei stride, size_t offset);
    void AddInputLayout(const std::string& name, GLint size, GLEnums::DATA_TYPE type, GLboolean normalized, GLsizei stride, size_t offset);

    template<typename... Rest>
    void SetDefaultInputLayout(const std::string& name, Rest... rest)
    {
        AddDefaultInputLayout(name);

        SetDefaultInputLayout(rest...);
    }

    void AddDefaultInputLayout(GLint location);
    void AddDefaultInputLayout(const std::string& name);

private:
    // Variadic template termination

    void SetDefaultInputLayout()
    { }

    template<typename T>
    void AppendInputLayout()
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Unknown input layout type!");
    }

    std::vector<std::pair<std::string, InputLayout>> namedInputLayouts;
    size_t vertexBuffer;
};

#endif // GLINPUTLAYOUT_H__
