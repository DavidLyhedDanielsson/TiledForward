#include "glShaderResourceBinds.h"
#include "glVertexShader.h"
#include "glPixelShader.h"
#include "logger.h"

#include <cstring>
#include <set>

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
        std::string name(maxAttribNameLength, '\0');
        GLint size;
        GLenum type;

        glGetActiveAttrib(shaderProgram, i, maxAttribNameLength, nullptr, &size, &type, &name[0]);
        name.resize(name.find_first_of('\0'));
        attributes.emplace_back(std::move(name), size, type);
    }

    return attributes;
}

std::vector<GLDrawBinds::Attrib> GLDrawBinds::GetActiveUniforms() const
{
    std::vector<Attrib> uniforms;

    GLint uniformCount;
    glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &uniformCount);

    GLint maxUniformNameLength;
    glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

    for(GLuint i = 0; i < uniformCount; ++i)
    {
        std::string name(maxUniformNameLength, '\0');
        GLint size;
        GLenum type;

        glGetActiveUniform(shaderProgram, i, maxUniformNameLength, nullptr, &size, &type, &name[0]);
        name.resize(name.find_first_of('\0'));
        uniforms.emplace_back(std::move(name), size, type);
    }

    return uniforms;
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
            LogWithName(LOG_TYPE::FATAL, "Type to number of float conversion failed for type " + std::to_string(type));
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

    CheckUniforms(GetActiveUniforms());

    //TODO: Optional input layout
    if(true)
    {
        if(vertexBuffers.size() != 1)
        {
            LogWithName(LOG_TYPE::WARNING, "Multiple vertex buffers bound to binds, automatic attrib enabling won't work");
            return false;
        }

        GLBufferLock bufferLock(vertexBuffers[0]);

        for(int i = 0; i < attributes.size(); ++i)
        {
            GLuint location = (GLuint)glGetAttribLocation(shaderProgram, attributes[i].name.c_str());

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

void GLDrawBinds::BindUniforms()
{
    for(const auto& pair : uniformBinds)
        pair.second->SetLocation(glGetUniformLocation(shaderProgram, pair.first.c_str()));
}

bool GLDrawBinds::CheckRequirements() const
{
    if(shaderBinds.size() == 0)
    {
        LogWithName(LOG_TYPE::FATAL, "No shaders added to binds");
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
            LogWithName(LOG_TYPE::WARNING, "No vertex shader added but a shader which might depend on it has been added");

        if(!computeBound)
        {
            LogWithName(LOG_TYPE::WARNING, "Neither a compute shader nor vertex shader has been added");
            return false;
        }
    }
    else
    {
        if(vertexBuffers.size() == 0)
        {
            LogWithName(LOG_TYPE::FATAL, "No vertex buffer added to binds with vertex shader");
            return false;
        }

        if(!pixelBound)
            LogWithName(LOG_TYPE::WARNING, "No pixel shader to match vertex shader, is this intended?");
    }

    return true;
}

bool GLDrawBinds::CheckUniforms(const std::vector<GLDrawBinds::Attrib>& activeUniforms)
{
    bool returnValue = true;

    std::set<std::string> usedUniforms;

    // TODO: Type checking!

    for(const Attrib& uniform : activeUniforms)
    {
        std::string actualName = uniform.name.substr(0, uniform.name.find_first_of("["));

        if(uniform.name.find_first_of(".") != uniform.name.npos)
            throw std::runtime_error("Implement this");

        if(uniformBinds.count(actualName) == 0)
            LogWithName(LOG_TYPE::DEBUG, "Unused uniform \"" + actualName + "\"");
        else
        {
            if(!uniformBinds.at(actualName)->VerifyType((GLEnums::UNIFORM_TYPE)uniform.type))
            {
                LogWithName(LOG_TYPE::WARNING, "Uniform \"" + actualName + "\" doesn't match shader type");
                return false;
            }

            usedUniforms.insert(actualName);
        }
    }

    for(const auto& pair : uniformBinds)
    {
        if(usedUniforms.count(pair.first) == 0)
            LogWithName(LOG_TYPE::DEBUG, "Bound uniform \"" + pair.first + "\" never used (is it optimized away?)");
    }

    return returnValue;
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
            if(pair.second->GetLocation() != -1)
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
        LogWithName(LOG_TYPE::FATAL, "DrawElements called without an index buffer set");
        return;
    }
#endif // NDEBUG

    glDrawElements(GL_TRIANGLES, indexBuffer->GetIndiciesCount(), GL_UNSIGNED_INT, (void*)0);
}

std::string GLDrawBinds::ToString() const
{
#ifndef NDEBUG
    std::string string = "Draw binds with:\n";

    std::string vertexShader;
    std::string tessControlShader;
    std::string tessEvalShader;
    std::string geometryShader;
    std::string pixelShader;
    std::string computeShader;

    for(const auto& pair : shaderBinds)
    {
        switch(pair.first)
        {
            case GLEnums::SHADER_TYPE::VERTEX:
                if(vertexShader.empty())
                    vertexShader = "Vertex shaders: ";

                vertexShader += pair.second->GetPath() + ", ";
                break;
            case GLEnums::SHADER_TYPE::TESS_CONTROL:
                if(tessControlShader.empty())
                    tessControlShader = "Tessellation control shaders: ";

                tessControlShader += pair.second->GetPath() + ", ";
                break;
            case GLEnums::SHADER_TYPE::TESS_EVALUATION:
                if(tessEvalShader.empty())
                    tessEvalShader = "Tessellation evaluation shaders: ";

                tessEvalShader += pair.second->GetPath() + ", ";
                break;
            case GLEnums::SHADER_TYPE::GEOMETRY:
                if(geometryShader.empty())
                    geometryShader = "Geometry shaders: ";

                geometryShader += pair.second->GetPath() + ", ";
                break;
            case GLEnums::SHADER_TYPE::PIXEL:
                if(pixelShader.empty())
                    pixelShader = "Pixel shaders: ";

                pixelShader += pair.second->GetPath() + ", ";
                break;
            case GLEnums::SHADER_TYPE::COMPUTE:
                if(computeShader.empty())
                    computeShader = "Compute shaders: ";

                computeShader += pair.second->GetPath() + ", ";
                break;
        }
    }

    if(!vertexShader.empty())
        string += vertexShader.substr(0, vertexShader.length() - 2) + "\n"; // Remove ", "
    if(!tessControlShader.empty())
        string += tessControlShader.substr(0, tessControlShader.length() - 2) + "\n"; // Remove ", "
    if(!tessEvalShader.empty())
        string += tessEvalShader.substr(0, tessEvalShader.length() - 2) + "\n"; // Remove ", "
    if(!geometryShader.empty())
        string += geometryShader.substr(0, geometryShader.length() - 2) + "\n"; // Remove ", "
    if(!pixelShader.empty())
        string += pixelShader.substr(0, pixelShader.length() - 2) + "\n"; // Remove ", "
    if(!computeShader.empty())
        string += computeShader.substr(0, computeShader.length() - 2) + "\n"; // Remove ", "

    return string;
#else // NDEBUG
    return "No shader names available in release";
#endif // NDEBUG
}

void GLDrawBinds::LogWithName(LOG_TYPE logType, const std::string& message) const
{
    Logger::LogLine(logType, ToString() + message);
}
