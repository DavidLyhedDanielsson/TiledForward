#include "glPixelShader.h"

#include <stdexcept>

GLPixelShader::GLPixelShader()
        : GLShader(GLEnums::SHADER_TYPE::PIXEL)
{}

GLPixelShader::~GLPixelShader()
{}