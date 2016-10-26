#include "glVertexShader.h"

GLVertexShader::GLVertexShader()
{}

GLVertexShader::~GLVertexShader()
{}

bool GLVertexShader::Load(const std::string& path)
{
    return GLShader::Load(GLEnums::SHADER_TYPE::VERTEX, path);
}
