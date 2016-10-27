#include "glShaderResourceBinds.h"
#include "glVertexShader.h"
#include "glPixelShader.h"
#include "logger.h"

#include <cstring>

GLDrawBinds::GLDrawBinds()
        : bound(false)
          , shaderProgram(0)
          , vao(0)
          , indexBuffer(nullptr)
{
}

GLDrawBinds::~GLDrawBinds()
{
    Unbind();

    if(shaderProgram != 0)
    {
        for(const auto& pair : shaderBinds)
            glDetachShader(shaderProgram, pair.second->GetShader());

        glDeleteProgram(shaderProgram);
    }

    for(const auto& pair : shaderBinds)
        delete pair.second;

    for(const auto& pair : uniformBinds)
        delete pair.second;

    if(vao != 0)
        glDeleteVertexArrays(1, &vao);
}

bool GLDrawBinds::Init()
{
    if(!CheckRequirements())
        return false;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    if(!CreateShaderProgram())
    {
        glBindVertexArray(0);
        glDeleteVertexArrays(1, &vao);

        return false;
    }

    glUseProgram(shaderProgram);
    BindUniforms();

    for(const auto& pair : uniformBinds)
        pair.second->UploadData();
    glUseProgram(0);

    glBindVertexArray(0);

    return true;
}

void GLDrawBinds::AddShader(GLEnums::SHADER_TYPE type, const std::string& shaderPath)
{
    GLShader* newShader;

    switch(type)
    {
        case GLEnums::SHADER_TYPE::VERTEX:
            newShader = new GLVertexShader();
            break;
        case GLEnums::SHADER_TYPE::PIXEL:
            newShader = new GLPixelShader();
            break;
        default:
            throw std::runtime_error("Shader type not implemented");
    }

    newShader->Load(shaderPath);

    shaderBinds.push_back(std::make_pair(type, newShader));
}

std::vector<GLDrawBinds::Attrib> GLDrawBinds::GetActiveAttribs() const
{
    std::vector<Attrib> attributes;

    GLint attribCount;
    glGetProgramiv(shaderProgram, GL_ACTIVE_ATTRIBUTES, &attribCount);

    GLint maxAttribNameLength;
    glGetProgramiv(shaderProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttribNameLength);

    for(GLuint i = 0; i < attribCount; ++i)
    {
        std::unique_ptr<GLchar> name(new GLchar[maxAttribNameLength]);
        GLint size;
        GLenum type;

        glGetActiveAttrib(shaderProgram, i, maxAttribNameLength, nullptr, &size, &type, name.get());
        attributes.emplace_back(std::move(name), size, type);
    }

    return attributes;
}

int GLDrawBinds::GetNumberOfFloats(GLenum type) const
{
    switch(type)
    {
        case GL_FLOAT:
            return 1;
        case GL_FLOAT_VEC2:
            return 2;
        case GL_FLOAT_VEC3:
            return 3;
        case GL_FLOAT_VEC4:
            return 4;
        case GL_FLOAT_MAT2:
            return 4;
        case GL_FLOAT_MAT3:
            return 9;
        case GL_FLOAT_MAT4:
            return 16;
        default:
            Logger::LogLine(LOG_TYPE::FATAL, "Type to number of float conversion failed for type ", type);
            return -1;
    }
}

bool GLDrawBinds::CreateShaderProgram()
{
    shaderProgram = glCreateProgram();

    for(const auto& pair : shaderBinds)
        glAttachShader(shaderProgram, pair.second->GetShader());

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

    std::vector<Attrib> attributes = GetActiveAttribs();

    //TODO: Optional input layout
    if(true)
    {
        if(vertexBuffers.size() != 1)
        {
            Logger::LogLine(LOG_TYPE::WARNING, "Multiple vertex buffers bound to binds, automatic attrib enabling won't work");
            return false;
        }

        GLBufferLock bufferLock(vertexBuffers[0]);

        for(int i = 0; i < attributes.size(); ++i)
        {
            GLuint location = (GLuint)glGetAttribLocation(shaderProgram, attributes[i].name.get());

            glEnableVertexAttribArray(location);
            glVertexAttribPointer(location
                                  , attributes[i].size * GetNumberOfFloats(attributes[i].type)
                                  , GL_FLOAT
                                  , GL_FALSE
                                  , vertexBuffers[0]->GetStride()
                                  , (void*)vertexBuffers[0]->GetOffsets()[location]);
        }
    }
    else
    {

    }

    glUseProgram(0);

    return true;
}

bool GLDrawBinds::BindUniforms()
{
    //TODO: Error handling
    for(const auto& pair : uniformBinds)
        pair.second->SetLocation(glGetUniformLocation(shaderProgram, pair.first.c_str()));
}

bool GLDrawBinds::CheckRequirements() const
{
    if(shaderBinds.size() == 0)
    {
        Logger::LogLine(LOG_TYPE::FATAL, "No shaders added to binds");
        return false;
    }

    bool vertexBound = false;
    bool tessControlBound = false;
    bool tessEvaluationBound = false;
    bool geometryBound = false;
    bool pixelBound = false;
    bool computeBound = false;

    for(const auto& pair : shaderBinds)
    {
        switch(pair.first)
        {
            case GLEnums::SHADER_TYPE::VERTEX:
                vertexBound = true;
                break;
            case GLEnums::SHADER_TYPE::TESS_CONTROL:
                tessControlBound = true;
                break;
            case GLEnums::SHADER_TYPE::TESS_EVALUATION:
                tessEvaluationBound = true;
                break;
            case GLEnums::SHADER_TYPE::GEOMETRY:
                geometryBound = true;
                break;
            case GLEnums::SHADER_TYPE::PIXEL:
                pixelBound = true;
                break;
            case GLEnums::SHADER_TYPE::COMPUTE:
                computeBound = true;
                break;
        }
    }

    if(!vertexBound)
    {
        if(tessControlBound || tessEvaluationBound || geometryBound || pixelBound)
            Logger::LogLine(LOG_TYPE::WARNING, "No vertex shader added but a shader which might depend on it has been added");

        if(!computeBound)
        {
            Logger::LogLine(LOG_TYPE::WARNING, "Neither a compute shader nor vertex shader has been added");
            return false;
        }
    }
    else
    {
        if(vertexBuffers.size() == 0)
        {
            Logger::LogLine(LOG_TYPE::FATAL, "No vertex buffer added to binds with vertex shader");
            return false;
        }

        if(!pixelBound)
        {
#ifndef NDEBUG
            GLShader* vertexShader = GetShader(GLEnums::SHADER_TYPE::VERTEX, 0);

            Logger::LogLine(LOG_TYPE::WARNING, "No pixel shader to match vertex shader \"" + vertexShader->GetPath() + "\", is this intended?");
#else
            Logger::LogLine(LOG_TYPE::WARNING, "No pixel shader to match vertex shader, is this intended?");
#endif // NDEBUG
        }
    }

    return true;
}

void GLDrawBinds::Bind()
{
    if(!bound)
    {
        bound = true;

        glBindVertexArray(vao);
        glUseProgram(shaderProgram);

        if(indexBuffer != nullptr)
            indexBuffer->Bind();

        for(const auto& vertexBuffer : vertexBuffers)
            vertexBuffer->Bind();

        for(const auto& pair : uniformBinds)
            pair.second->UploadData();
    }
}

void GLDrawBinds::Unbind()
{
    if(bound)
    {
        bound = false;

        glBindVertexArray(0);
        glUseProgram(0);

        if(indexBuffer != nullptr)
            indexBuffer->Unbind();

        for(const auto& vertexBuffer : vertexBuffers)
            vertexBuffer->Unbind();
    }
}

GLShader* GLDrawBinds::GetShader(GLEnums::SHADER_TYPE type, int index) const
{
    int currentIndex = 0;

    for(const auto& pair : shaderBinds)
    {
        if(pair.first == type)
        {
            if(currentIndex == index)
                return pair.second;
            else
                ++currentIndex;
        }
    }

    return nullptr;
}

int GLDrawBinds::GetShaderTypeCount(GLEnums::SHADER_TYPE type) const
{
    int count = 0;

    for(const auto& pair : shaderBinds)
    {
        if(pair.first == type)
            ++count;
    }

    return count;
}

void GLDrawBinds::DrawElements()
{
#ifndef NDEBUG
    if(indexBuffer == nullptr)
    {
        Logger::LogLine(LOG_TYPE::FATAL, "DrawElements called without an index buffer set");
        return;
    }
#endif // NDEBUG

    glDrawElements(GL_TRIANGLES, indexBuffer->GetIndiciesCount(), GL_UNSIGNED_INT, (void*)0);
}
