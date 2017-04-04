#include "glDrawBinds.h"
#include "glVertexShader.h"
#include "glPixelShader.h"
#include "../logger.h"
#include "../content/shaderContentParameters.h"

#include <cstring>
#include <set>
#include <algorithm>

GLDrawBinds::GLDrawBinds()
        : bound(false)
          , shaderProgram(0)
          , vao(0)
          , indexBuffer(nullptr)
          , currentBindingPoint(0)
{
}

GLDrawBinds::~GLDrawBinds()
{
    Unbind();

    if(shaderProgram != 0)
    {
        // If the shaders are unloaded first glDetachShader will be called from there
        // and the shader will be set to 0
        for(const auto& pair : shaderBinds)
            if(pair.second->GetShader() != 0)
                glDetachShader(shaderProgram, pair.second->GetShader());

        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    if(vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }
}

bool GLDrawBinds::Init()
{
    if(CheckRequirements())
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        indexBuffer->Bind();
    }

    if(!CreateShaderProgram())
    {
        if(vao != 0)
        {
            glBindVertexArray(0);
            glDeleteVertexArrays(1, &vao);
        }

        return false;
    }

    glUseProgram(shaderProgram);
    BindUniforms();

    for(const auto& pair : uniformBinds)
        pair.second->UploadData();

    if(vao != 0)
    {
        glBindVertexArray(0);
        glUseProgram(0);

        indexBuffer->Unbind();
    }

    return true;
}

bool GLDrawBinds::AddShader(ContentManager& contentManager
                            , GLEnums::SHADER_TYPE type
                            , const std::string& shaderPath)
{
    ShaderContentParameters parameters(type);

    GLShader* newShader = contentManager.Load<GLShader>(shaderPath, &parameters);
    if(newShader == nullptr)
        return false;

    shaderBinds.push_back(std::make_pair(type, newShader));

    return true;
}

bool GLDrawBinds::AddShader(ContentManager& contentManager
                            , ShaderContentParameters& parameters
                            , const std::string& shaderPath)
{
    GLShader* newShader = contentManager.Load<GLShader>(shaderPath, &parameters);
    if(newShader == nullptr)
        return false;

    shaderBinds.push_back(std::make_pair(parameters.type, newShader));

    return true;
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
            return 0;
    }
}

bool GLDrawBinds::CreateShaderProgram()
{
    //TODO: Cleanup on return false
    shaderProgram = glCreateProgram();

    for(auto& pair : shaderBinds)
    {
        pair.second->shaderPrograms.push_back(this);
        glAttachShader(shaderProgram, pair.second->GetShader());
    }

    glLinkProgram(shaderProgram);

    GLint linkSuccess = 0;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkSuccess);

    if(linkSuccess != GL_TRUE)
    {
        GLint logLength = 0;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);

        if(shaderBinds.size() != 0)
        {
            GLint shaderSourceLength;
            glGetShaderiv(shaderBinds.front().second->GetShader(), GL_SHADER_SOURCE_LENGTH, &shaderSourceLength);
            std::string shaderSource(shaderSourceLength, -1);
            glGetShaderSource(shaderBinds.front().second->GetShader(), shaderSourceLength, nullptr, (GLchar*)shaderSource.data());

            std::unique_ptr<GLchar> log(new GLchar[logLength]);
            glGetProgramInfoLog(shaderProgram, logLength, nullptr, log.get());
            LogWithName(LOG_TYPE::FATAL, std::string("Couldn't link shader program: ") + const_cast<const char*>(log.get()) + "\nShader source:\n" + shaderSource);
        }
        else
        {
            std::unique_ptr<GLchar> log(new GLchar[logLength]);
            glGetProgramInfoLog(shaderProgram, logLength, nullptr, log.get());
            LogWithName(LOG_TYPE::FATAL, std::string("Couldn't link shader program: ") + const_cast<const char*>(log.get()));
        }

        return false;
    }

    glUseProgram(shaderProgram);

    if(!CheckUniforms(GetActiveUniforms()))
        return false;

    std::vector<Attrib> attributes = GetActiveAttribs();

    if(!vertexBuffers.empty())
    {
        if(inputLayouts.size() == 0)
        {
            // Create default input layouts

            if(vertexBuffers.size() != 1)
            {
                LogWithName(LOG_TYPE::FATAL, "Multiple vertex buffers bound to drawBinds, automatic attrib enabling won't work");
                return false;
            }

#ifndef NDEBUG
            int numberOfFloats = 0;
            for(const auto& attrib : attributes)
                numberOfFloats += GetNumberOfFloats(attrib.type);

            if(numberOfFloats * sizeof(float) != vertexBuffers[0]->GetStride())
                LogWithName(LOG_TYPE::DEBUG, "Vertex buffer stride isn't equal to the size of all attributes, is this intended?");
#endif // NDEBUG

            GLBufferLock bufferLock(vertexBuffers[0]);

            for(int i = 0; i < attributes.size(); ++i)
            {
                GLuint location = (GLuint)glGetAttribLocation(shaderProgram, attributes[i].name.c_str());

                glEnableVertexAttribArray(location);

                if(vertexBuffers[0]->GetOffsets().empty())
                    glVertexAttribPointer(location
                                          , attributes[i].size * GetNumberOfFloats(attributes[i].type)
                                          , GL_FLOAT
                                          , GL_FALSE
                                          , 0
                                          , 0);
                else
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
            // User has defined input layouts
#ifndef NDEBUG

            int numberOfFloats = 0;
            for(const auto& attrib : attributes)
                numberOfFloats += GetNumberOfFloats(attrib.type);

            //if(vertexBuffers[0]->GetStride() != numberOfFloats * sizeof(float)) // TODO: Multiple buffers
            //    LogWithName(LOG_TYPE::DEBUG, "Vertex buffer stride isn't equal to the size of all attributes (" + std::to_string(vertexBuffers[0]->GetStride()) + " != " + std::to_string(numberOfFloats * sizeof(float)) + ") is this intended?");

            // TODO: Count size of input layouts?
#endif // NDEBUG

            for(int i = 0; i < inputLayouts.size(); ++i)
            {
                const GLInputLayout& inputLayout = inputLayouts[i];

                // TODO: Make sure every attribute has a layout
                for(int j = 0; j < inputLayout.namedInputLayouts.size(); ++j)
                {
                    const std::pair<std::string, InputLayout>& current = inputLayout.namedInputLayouts[j];

                    GLuint location = (GLuint)(std::isdigit(current.first[0]) ? std::stoi(current.first) : glGetAttribLocation(shaderProgram, current.first.c_str()));

                    if(location == -1)
                    {
                        LogWithName(LOG_TYPE::DEBUG, "Attribute " + current.first + " not found in shader (is it optimized away?)");
                        continue;
                    }

                    GLint size = current.second.size == -1 ? attributes[j].size * GetNumberOfFloats(attributes[j].type) : current.second.size;
                    GLenum type = current.second.type == GLEnums::DATA_TYPE::UNKNOWN ? GL_FLOAT : (GLenum)current.second.type;
                    GLboolean normalized = current.second.normalized;
                    GLsizei stride = current.second.stride == -1 ? vertexBuffers[inputLayout.vertexBuffer]->GetStride() : current.second.stride;
                    void* offset = (void*)(current.second.offset == -1 ? vertexBuffers[inputLayout.vertexBuffer]->GetOffsets()[location] : current.second.offset);

                    GLBufferLock bufferLock(vertexBuffers[inputLayout.vertexBuffer]);

                    glEnableVertexAttribArray(location);
                    glVertexAttribPointer(location
                                          , size
                                          , type
                                          , normalized
                                          , stride
                                          , offset);

                    if(current.second.vertexAttribDivisor != -1)
                        glVertexAttribDivisor(location, current.second.vertexAttribDivisor);
                }
            }
        }
    }

    glUseProgram(0);

    return true;
}

void GLDrawBinds::BindUniforms()
{
    //for(const auto& pair : uniformBinds)
    for(auto iter = uniformBinds.begin(), end = uniformBinds.end(); iter != end; ++iter)
    {
        GLint location = glGetUniformLocation(shaderProgram, iter->first.c_str());
        if(location == -1)
        {
            Logger::LogLine(LOG_TYPE::WARNING, "Trying to bind nonexistent uniform \"" +
                                               iter->first +
                                               "\", nothing will be bound");

            //iter = uniformBinds.erase(iter);
        }
        else
            iter->second->SetLocation(location);
    }
}

bool GLDrawBinds::CheckRequirements() const
{
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
            case GLEnums::SHADER_TYPE::FRAGMENT:
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
            LogWithName(LOG_TYPE::FATAL, "No vertex buffer added to drawBinds with vertex shader");
            return false;
        }

        //if(!pixelBound)
        //    LogWithName(LOG_TYPE::WARNING, "No pixel shader to match vertex shader, is this intended?");
    }

    return !computeBound;
}

bool GLDrawBinds::CheckUniforms(const std::vector<GLDrawBinds::Attrib>& activeUniforms)
{
    bool returnValue = true;

    std::set<std::string> usedUniforms;

    GetActiveStorageBlocks();
    GetActiveUniformBlocks();

    for(const Attrib& uniform : activeUniforms)
    {
        bool insideBlock = false;

        for(const auto& pair : uniformBufferBinds)
        {
            if(pair.second->VariableExists(uniform.name))
            {
                insideBlock = true;
                break;
            }
        }

        if(insideBlock)
            continue;

        std::string actualName = uniform.name.substr(0, uniform.name.find_first_of("["));

        if(uniform.name.find_first_of(".") != uniform.name.npos)
        {
            // TODO
            continue;
        }

        if(uniformBinds.count(actualName) == 0)
        {
            // TODO: Uncomment
            //LogWithName(LOG_TYPE::DEBUG, "Uniform \"" + actualName + "\" is never used");
        }
        else
        {
            if(!uniformBinds.at(actualName)->VerifyType((GLEnums::UNIFORM_TYPE)uniform.type))
            {
                LogWithName(LOG_TYPE::WARNING, "Uniform \"" + actualName + "\" doesn't match shader type");
                returnValue = false;
            }

            usedUniforms.insert(actualName);
        }
    }

    for(const auto& pair : uniformBinds)
    {
        if(usedUniforms.count(pair.first) == 0)
            LogWithName(LOG_TYPE::DEBUG, "Bound uniform \"" + pair.first + "\" is never used (is it optimized away?)");
    }

    return returnValue;
}

void GLDrawBinds::GetActiveStorageBlocks()
{
    GLint count;
    glGetProgramInterfaceiv(shaderProgram, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES, &count);

    GLint maxNameLength;
    glGetProgramInterfaceiv(shaderProgram, GL_SHADER_STORAGE_BLOCK, GL_MAX_NAME_LENGTH, &maxNameLength);

    for(int i = 0; i < count; ++i)
    {
        std::string name(maxNameLength, '\0');

        GLsizei writtenLength;
        glGetProgramResourceName(shaderProgram, GL_SHADER_STORAGE_BLOCK, i, maxNameLength, &writtenLength, &name[0]);

        name.erase(writtenLength);

        int blockIndex = glGetProgramResourceIndex(shaderProgram, GL_SHADER_STORAGE_BLOCK, &name[0]);

        if(blockIndex == -1)
        {
            Logger::LogLine(LOG_TYPE::WARNING, "Shader storage block \"" + name + "\" used in shader but not bound to draw binds");
            continue;
        }

        if(storageBufferBinds.count(name) == 0)
        {
            storageBufferBinds.insert(make_pair(name, std::unique_ptr<GLShaderStorageBuffer>(new GLShaderStorageBuffer(name
                                                                                                                       , shaderProgram
                                                                                                                       , blockIndex
                                                                                                                       , currentBindingPoint))));
            storageBufferBinds.at(name)->Init();
            currentBindingPoint++;
        }
        else
        {
            storageBufferBinds.at(name)->shaderProgram = shaderProgram;
            storageBufferBinds.at(name)->blockIndex = blockIndex;

            // TODO: Call Init?
            glShaderStorageBlockBinding(shaderProgram, blockIndex, storageBufferBinds.at(name)->bindingPoint);
        }
    }
}

void GLDrawBinds::GetActiveUniformBlocks()
{
    GLint count;
    glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORM_BLOCKS, &count);

    GLint maxNameLength;
    glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &maxNameLength);

    for(int i = 0; i < count; ++i)
    {
        std::string name(maxNameLength, '\0');

        GLsizei writtenLength;
        glGetActiveUniformBlockName(shaderProgram, i, maxNameLength, &writtenLength, &name[0]);

        name.erase(writtenLength);

        int blockIndex = glGetProgramResourceIndex(shaderProgram, GL_UNIFORM_BLOCK, &name[0]);

        if(uniformBufferBinds.count(name) == 0)
        {
            uniformBufferBinds.insert(make_pair(name, std::unique_ptr<GLUniformBuffer>(new GLUniformBuffer(name
                                                                                                           , shaderProgram
                                                                                                           , blockIndex
                                                                                                           , currentBindingPoint))));
            uniformBufferBinds.at(name)->Init();
            currentBindingPoint++;
        }
        else
        {
            uniformBufferBinds.at(name)->shaderProgram = shaderProgram;
            uniformBufferBinds.at(name)->blockIndex = blockIndex;

            // TODO: Call Init?
            glUniformBlockBinding(shaderProgram, blockIndex, uniformBufferBinds.at(name)->bindingPoint);
        }
    }
}

void GLDrawBinds::Bind()
{
    if(!bound)
    {
        bound = true;

        glBindVertexArray(vao);
        glUseProgram(shaderProgram);

        //if(indexBuffer != nullptr)
        //    indexBuffer->Bind();

        //for(const auto& vertexBuffer : vertexBuffers)
        //    vertexBuffer->Bind();

        for(const auto& pair : uniformBinds)
            if(pair.second->GetLocation() != -1)
                pair.second->UploadData();

        for(const auto& pair : uniformBufferBinds)
            pair.second->Bind();

        for(const auto& pair : storageBufferBinds)
            pair.second->Bind();
    }
}

void GLDrawBinds::Unbind()
{
    if(bound)
    {
        bound = false;

        glBindVertexArray(0);
        glUseProgram(0);

        //if(indexBuffer != nullptr)
        //    indexBuffer->Unbind();

        //for(const auto& vertexBuffer : vertexBuffers)
        //    vertexBuffer->Unbind();

        for(const auto& pair : uniformBufferBinds)
            pair.second->Unbind();

        for(const auto& pair : storageBufferBinds)
            pair.second->Unbind();
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

void GLDrawBinds::DrawElements(GLEnums::DRAW_MODE drawMode /*= GLEnums::DRAW_MODE::TRIANGLES*/)
{
#ifndef NDEBUG
    if(indexBuffer == nullptr)
    {
        LogWithName(LOG_TYPE::FATAL, "DrawElements called without an blockIndex buffer set");
        return;
    }
#endif // NDEBUG

    glDrawElements((GLenum)drawMode, indexBuffer->GetIndiciesCount(), GL_UNSIGNED_INT, (void*)0);
}

void GLDrawBinds::DrawElements(GLsizei count, GLEnums::DRAW_MODE drawMode /*= GLEnums::DRAW_MODE::TRIANGLES*/)
{
#ifndef NDEBUG
    if(indexBuffer == nullptr)
    {
        LogWithName(LOG_TYPE::FATAL, "No blockIndex buffer set");
        return;
    }

    if(count > indexBuffer->GetIndiciesCount())
    {
        LogWithName(LOG_TYPE::WARNING, "count is bigger than number of indicies in buffer (" +
                                     std::to_string(count) +
                                     " > " +
                                     std::to_string(indexBuffer->GetIndiciesCount()) +
                                     "). count will be clamped");

        count = indexBuffer->GetIndiciesCount();
    }
#endif // NDEBUG

    glDrawElements((GLenum)drawMode, count, GL_UNSIGNED_INT, (void*)0);
}

void GLDrawBinds::DrawElements(GLsizei count, GLsizei offset, GLEnums::DRAW_MODE drawMode /*= GLEnums::DRAW_MODE::TRIANGLES*/)
{
#ifndef NDEBUG
    if(indexBuffer == nullptr)
    {
        LogWithName(LOG_TYPE::FATAL, "DrawElements called without an blockIndex buffer set");
        return;
    }

    if(offset + count > indexBuffer->GetIndiciesCount())
    {
        LogWithName(LOG_TYPE::WARNING, "offset + count is bigger than number of indicies in buffer ("
                                       + std::to_string(offset)
                                       + " + "
                                       + std::to_string(count)
                                       + " > "
                                       + std::to_string(indexBuffer->GetIndiciesCount())
                                       + "). count will be clamped");

        count = indexBuffer->GetIndiciesCount() - offset;
    }
#endif // NDEBUG

    glDrawElements((GLenum)drawMode, count, GL_UNSIGNED_INT, (void*)(offset * sizeof(GLuint)));
}

void GLDrawBinds::DrawElementsInstanced(int instances, GLEnums::DRAW_MODE drawMode /*= GLEnums::DRAW_MODE::TRIANGLES*/)
{
#ifndef NDEBUG
    if(indexBuffer == nullptr)
    {
        LogWithName(LOG_TYPE::FATAL, "DrawElementsInstanced called without an blockIndex buffer set");
        return;
    }
#endif // NDEBUG

    glDrawElementsInstanced((GLenum)drawMode, indexBuffer->GetIndiciesCount(), GL_UNSIGNED_INT, (void*)0, instances);
}

void GLDrawBinds::AddBuffer(GLVertexBuffer* vertexBuffer)
{
    vertexBuffers.push_back(vertexBuffer);
}

void GLDrawBinds::AddBuffer(GLVertexBuffer* vertexBuffer, GLInputLayout inputLayout)
{
    if(vertexBuffers.size() != inputLayouts.size())
    {
        Logger::LogLine(LOG_TYPE::WARNING
                    , "Not all vertex buffers given an input layout, default input layouts will be created");

        while(vertexBuffers.size() > inputLayouts.size())
        {
            GLInputLayout layout;
            layout.vertexBuffer = inputLayouts.size(); // TODO: Test this
            inputLayouts.push_back(layout);
        }
    }

    inputLayout.vertexBuffer = vertexBuffers.size();
    vertexBuffers.push_back(vertexBuffer);
    inputLayouts.push_back(inputLayout);
}

void GLDrawBinds::AddBuffer(GLIndexBuffer* indexBuffer)
{
#ifndef NDEBUG
    if(this->indexBuffer != nullptr)
        Logger::LogLine(LOG_TYPE::WARNING, "Overwriting blockIndex buffer");
#endif // NDEBUG

    this->indexBuffer = indexBuffer;
}

std::string GLDrawBinds::ToString() const
{
#ifndef NDEBUG
    std::string string = "Draw drawBinds with:\n";

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

                (vertexShader += pair.second->GetPath()) += ", ";
                break;
            case GLEnums::SHADER_TYPE::TESS_CONTROL:
                if(tessControlShader.empty())
                    tessControlShader = "Tessellation control shaders: ";

                (tessControlShader += pair.second->GetPath()) += ", ";
                break;
            case GLEnums::SHADER_TYPE::TESS_EVALUATION:
                if(tessEvalShader.empty())
                    tessEvalShader = "Tessellation evaluation shaders: ";

                (tessEvalShader += pair.second->GetPath()) += ", ";
                break;
            case GLEnums::SHADER_TYPE::GEOMETRY:
                if(geometryShader.empty())
                    geometryShader = "Geometry shaders: ";

                (geometryShader += pair.second->GetPath()) += ", ";
                break;
            case GLEnums::SHADER_TYPE::FRAGMENT:
                if(pixelShader.empty())
                    pixelShader = "Pixel shaders: ";

                (pixelShader += pair.second->GetPath()) += ", ";
                break;
            case GLEnums::SHADER_TYPE::COMPUTE:
                if(computeShader.empty())
                    computeShader = "Compute shaders: ";

                (computeShader += pair.second->GetPath()) += ", ";
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

void GLDrawBinds::RelinkShaders()
{
    glLinkProgram(shaderProgram);

    glUseProgram(shaderProgram);

    CheckUniforms(GetActiveUniforms());
    BindUniforms();

    glUseProgram(0);
}

GLuint GLDrawBinds::GetShaderProgram() const
{
    return shaderProgram;
}

bool GLDrawBinds::IsBound() const
{
    return bound;
}

GLVariable GLDrawBinds::operator[](const std::string& name)
{
    if(uniformBinds.count(name) != 0)
        return GLVariable(this, uniformBinds[name].get());
    else if(uniformBufferBinds.count(name) != 0)
        return GLVariable(this, uniformBufferBinds[name].get());
    else if(storageBufferBinds.count(name) != 0)
        return GLVariable(this, storageBufferBinds[name].get());

    if(alreadyWarned.count(name) == 0)
    {
        Logger::LogLine(LOG_TYPE::DEBUG, "Trying to get uniform \""
                                         + name
                                         + "\" which doesn't exist! Did you forget to call AddUniform?");

        alreadyWarned.insert(name);
    }
    return GLVariable();
}

GLShaderStorageBuffer* GLDrawBinds::GetSSBO(const std::string& name)
{
    if(storageBufferBinds.count(name) != 0)
        return storageBufferBinds.at(name).get();

    return nullptr;
}

GLUniformBuffer* GLDrawBinds::GetUBO(const std::string& name)
{
    if(uniformBufferBinds.count(name) != 0)
        return uniformBufferBinds.at(name).get();

    return nullptr;
}

void GLDrawBinds::Share(GLShaderStorageBuffer* lhs, GLShaderStorageBuffer* rhs)
{
    lhs->Share(rhs);
}

void GLDrawBinds::Share(GLUniformBuffer* lhs, GLUniformBuffer* rhs)
{
    lhs->Share(rhs);
}

bool GLDrawBinds::ChangeShader(ContentManager& contentManager
                               , GLEnums::SHADER_TYPE shaderType
                               , const std::string& newShaderPath)
{
    ShaderContentParameters parameters(shaderType);
    GLShader* newShader = contentManager.Load<GLShader>(newShaderPath, &parameters);
    if(newShader == nullptr)
        return false;

    for(auto& pair : shaderBinds)
    {
        auto iter = std::find(pair.second->shaderPrograms.begin(), pair.second->shaderPrograms.end(), this);
        if(iter != pair.second->shaderPrograms.end())
            pair.second->shaderPrograms.erase(iter);
        glDetachShader(shaderProgram, pair.second->GetShader());

        if(pair.first == shaderType)
            pair.second = newShader;
    }

    Unbind();

    if(shaderProgram != 0)
    {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    if(vao != 0)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    return Init();
}

void GLVariable::operator=(const GLVariable& rhs)
{
    if(uniformBuffer != nullptr && rhs.uniformBuffer != nullptr)
    {
        parent->Share(uniformBuffer, rhs.uniformBuffer);
    }
    else if(storageBuffer != nullptr && rhs.storageBuffer != nullptr)
    {
        parent->Share(storageBuffer, rhs.storageBuffer);
    }
}