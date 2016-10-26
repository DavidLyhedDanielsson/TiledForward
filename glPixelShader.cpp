#include "glPixelShader.h"

#include <stdexcept>

GLPixelShader::GLPixelShader()
{}

GLPixelShader::~GLPixelShader()
{}

bool GLPixelShader::Load(const std::string& path)
{
    return GLShader::Load(GLEnums::SHADER_TYPE::PIXEL, path);
}

bool GLPixelShader::Load(const std::string& path, const std::string& outBuffer)
{
    throw std::runtime_error("Not implemented");
}
