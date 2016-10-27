//#include "glShaderProgram.h"
//
//GLShaderProgram::GLShaderProgram()
//        : shaderProgram(0)
//{}
//
//GLShaderProgram::~GLShaderProgram()
//{
//
//}
//
//bool GLShaderProgram::Load(std::vector<InputLayout> inputLayout, GLShader* vertexShader, GLShader* pixelShader)
//{
//    shaders.push_back(std::make_pair(GLEnums::SHADER_TYPE::VERTEX, vertexShader));
//    shaders.push_back(std::make_pair(GLEnums::SHADER_TYPE::PIXEL, pixelShader));
//
//
//
//    return true;
//}
//
//void GLShaderProgram::Bind()
//{
//    glBindVertexArray(vao);
//    glUseProgram(shaderProgram);
//}
//
//void GLShaderProgram::Unbind()
//{
//    glBindVertexArray(0);
//    glUseProgram(0);
//}
