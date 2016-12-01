#include "glInputLayout.h"

GLInputLayout::GLInputLayout()
{}

GLInputLayout::~GLInputLayout()
{}

void GLInputLayout::AddInputLayout(GLint location
                                   , GLint size
                                   , GLEnums::DATA_TYPE type
                                   , GLboolean normalized
                                   , GLsizei stride
                                   , size_t offset)
{
    namedInputLayouts.push_back(std::make_pair(std::to_string(location), InputLayout{ size, type, normalized, stride, offset }));
}

void GLInputLayout::AddInputLayout(const std::string& name
                                   , GLint size
                                   , GLEnums::DATA_TYPE type
                                   , GLboolean normalized
                                   , GLsizei stride
                                   , size_t offset)
{
    namedInputLayouts.push_back(std::make_pair(name, InputLayout{ size, type, normalized, stride, offset }));
}

void GLInputLayout::AddDefaultInputLayout(GLint location)
{
    AddInputLayout(location, -1, GLEnums::DATA_TYPE::UNKNOWN, GL_FALSE, -1, -1);
}

void GLInputLayout::AddDefaultInputLayout(const std::string& name)
{
    AddInputLayout(name, -1, GLEnums::DATA_TYPE::UNKNOWN, GL_FALSE, -1, -1);
}
