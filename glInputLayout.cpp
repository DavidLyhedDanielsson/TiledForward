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
                                   , size_t offset
                                   , GLint vertexDivisor)
{
    namedInputLayouts.push_back(std::make_pair(std::to_string(location), InputLayout(size, type, normalized, stride, offset, vertexDivisor)));
}

void GLInputLayout::AddInputLayout(const std::string& name
                                   , GLint size
                                   , GLEnums::DATA_TYPE type
                                   , GLboolean normalized
                                   , GLsizei stride
                                   , size_t offset
                                   , GLint vertexDivisor)
{
    namedInputLayouts.push_back(std::make_pair(name, InputLayout(size, type, normalized, stride, offset, vertexDivisor)));
}

void GLInputLayout::AddDefaultInputLayout(GLint location)
{
    AddInputLayout(location, -1, GLEnums::DATA_TYPE::UNKNOWN, GL_FALSE, -1, -1, 0);
}

void GLInputLayout::AddDefaultInputLayout(const std::string& name)
{
    AddInputLayout(name, -1, GLEnums::DATA_TYPE::UNKNOWN, GL_FALSE, -1, -1, 0);
}

void GLInputLayout::SetVertexAttribDivisor(int location, int divisor)
{
    for(auto& pair : namedInputLayouts)
    {
        if(pair.first == std::to_string(location))
        {
            pair.second.vertexAttribDivisor = divisor;
            return;
        }
    }

    Logger::LogLine(LOG_TYPE::DEBUG, "Trying to set vertex attrib divisor on non-existent location \"", location, "\"");
}
