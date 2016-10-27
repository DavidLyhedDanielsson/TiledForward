#ifndef GLSHADER_H__
#define GLSHADER_H__

#include <string>

#include "glEnums.h"

class GLShader
{
public:
    GLShader(GLEnums::SHADER_TYPE shaderType);
    virtual ~GLShader();

    bool Load(const std::string& path);

    GLEnums::SHADER_TYPE GetShaderType() const;
    GLuint GetShader() const;

#ifndef NDEBUG
    const std::string& GetPath() const;
#endif // NDEBUG

protected:
private:
#ifndef NDEBUG
    std::string path;
#endif // NDEBUG

    GLEnums::SHADER_TYPE shaderType;
    GLuint shader;
};

#endif // GLSHADER_H__
