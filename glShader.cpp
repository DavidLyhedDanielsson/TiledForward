#include <fstream>
#include <memory>
#include "glShader.h"
#include "logger.h"

GLShader::GLShader()
        : shader(0)
{}

GLShader::~GLShader()
{
    if(shader != 0)
        glDeleteShader(shader);
}

bool GLShader::Load(GLEnums::SHADER_TYPE shaderType, const std::string& path)
{
    std::ifstream in(path, std::ios::ate);
    if(!in.is_open())
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't open file at \"" + path + "\"");
        return false;
    }

    std::string shaderSource;
    shaderSource.resize(in.tellg(), 0);
    in.seekg(std::ios::beg);
    in.read(&shaderSource[0], shaderSource.size());

    shader = glCreateShader((GLuint)shaderType);
    GLchar const* data = &shaderSource[0];
    glShaderSource(shader, 1, &data, NULL);
    glCompileShader(shader);

    GLint compileStatus;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
    if(compileStatus != GL_TRUE)
    {
        GLint logSize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

        std::unique_ptr<GLchar> errorLog(new char[logSize]);
        glGetShaderInfoLog(shader, logSize, nullptr, errorLog.get());

        Logger::LogLine(LOG_TYPE::FATAL, "Error when trying to load shader at \"" + path + "\": ", const_cast<const char*>(errorLog.get()));

        return false;
    }

    return true;
}

GLuint GLShader::GetShader() const
{
    return shader;
}
