#include <glm/mat4x4.hpp>
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

template<>
void GLInputLayout::AppendInputLayout<int>()
{
    AddInputLayout((int)namedInputLayouts.size(), 1, GLEnums::DATA_TYPE::INT, GL_FALSE, -1, -1);
}

template<>
void GLInputLayout::AppendInputLayout<float>()
{
    AddInputLayout((int)namedInputLayouts.size(), 1, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1);
}

template<>
void GLInputLayout::AppendInputLayout<glm::vec2>()
{
    AddInputLayout((int)namedInputLayouts.size(), 2, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1);
}

template<>
void GLInputLayout::AppendInputLayout<glm::vec3>()
{
    AddInputLayout((int)namedInputLayouts.size(), 3, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1);
}

template<>
void GLInputLayout::AppendInputLayout<glm::vec4>()
{
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1);
}

template<>
void GLInputLayout::AppendInputLayout<glm::mat4x4>()
{
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1);
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1);
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1);
    AddInputLayout((int)namedInputLayouts.size(), 4, GLEnums::DATA_TYPE::FLOAT, GL_FALSE, -1, -1);
}