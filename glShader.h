#ifndef GLSHADER_H__
#define GLSHADER_H__

#include <string>

#include "glEnums.h"

class GLShader
{
public:
    GLShader();
    virtual ~GLShader();

    bool Load(GLEnums::SHADER_TYPE shaderType, const std::string& path);

    GLuint GetShader() const;

protected:
private:
    GLuint shader;
};

#endif // GLSHADER_H__
