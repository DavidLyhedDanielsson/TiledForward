#include "glUniformBlock.h"

GLUniformBlock::GLUniformBlock(const std::string& name)
        : name(name)
{}

GLUniformBlock::~GLUniformBlock()
{
    for(const auto& pair : uniforms)
        delete pair.second;

    uniforms.clear();
}