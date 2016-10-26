#include "glShaderProgram.h"

GLShaderProgram::GLShaderProgram()
        : shaderProgram(0)
{}

GLShaderProgram::~GLShaderProgram()
{
    if(shaderProgram != 0)
    {
        for(const auto& pair : shaders)
            glDetachShader(shaderProgram, pair.second->GetShader());

        glDeleteProgram(shaderProgram);
    }
}

bool GLShaderProgram::Load(std::vector<InputLayout> inputLayout, GLShader* vertexShader, GLShader* pixelShader)
{
    shaders.push_back(std::make_pair(GLEnums::SHADER_TYPE::VERTEX, vertexShader));
    shaders.push_back(std::make_pair(GLEnums::SHADER_TYPE::PIXEL, pixelShader));

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader->GetShader());
    glAttachShader(shaderProgram, pixelShader->GetShader());

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    for(const auto& input : inputLayout)
    {
        glEnableVertexAttribArray(std::get<0>(input));
        glVertexAttribPointer(std::get<0>(input), std::get<1>(input), std::get<2>(input), std::get<3>(input), std::get<4>(input), reinterpret_cast<void*>(std::get<5>(input)));
    }

    glBindVertexArray(0);

    glUseProgram(0);

    return true;
}

void GLShaderProgram::Bind()
{
    glBindVertexArray(vao);
    glUseProgram(shaderProgram);
}

void GLShaderProgram::Unbind()
{
    glBindVertexArray(0);
    glUseProgram(0);
}
