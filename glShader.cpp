#include <fstream>
#include <memory>
#include "glShader.h"
#include "logger.h"
#include "shaderContentParameters.h"

#include "glDrawBinds.h"
#include "glUniformBlock.h"

GLShader::GLShader()
        : shaderType(GLEnums::SHADER_TYPE::UNKNOWN)
          , shader(0)
{}

GLShader::GLShader(GLEnums::SHADER_TYPE type)
        : shaderType(type)
{}

GLShader::~GLShader()
{ }

GLEnums::SHADER_TYPE GLShader::GetShaderType() const
{
    return shaderType;
}

GLuint GLShader::GetShader() const
{
    return shader;
}

bool GLShader::CreateDefaultContent(const char* filePath, ContentManager* contentManager)
{
    std::string shaderSource;

    switch(shaderType)
    {
        case GLEnums::SHADER_TYPE::VERTEX:
        case GLEnums::SHADER_TYPE::PIXEL:
            shaderSource = "void main() { }";
            break;
        case GLEnums::SHADER_TYPE::TESS_CONTROL: // TODO: Implement these
        case GLEnums::SHADER_TYPE::TESS_EVALUATION:
        case GLEnums::SHADER_TYPE::GEOMETRY:
        case GLEnums::SHADER_TYPE::COMPUTE:
        case GLEnums::SHADER_TYPE::UNKNOWN:
            return false;
    }

    return CompileFromSource(shaderSource);
}

bool GLShader::Apply(Content* content)
{
    GLShader* shader = dynamic_cast<GLShader*>(content);
    if(!shader)
        return false;

    this->shader = shader->shader;

    // TODO: Move?
    for(auto shaderProgram : shaderPrograms)
        glAttachShader(shaderProgram->GetShaderProgram(), this->shader);

    for(auto shaderProgram : shaderPrograms)
        shaderProgram->RelinkShaders();
}

CONTENT_ERROR_CODES GLShader::Load(const char* filePath
                                   , ContentManager* contentManager
                                   , ContentParameters* contentParameters)
{
    ShaderContentParameters* parameters = TryCastTo<ShaderContentParameters>(contentParameters);
    if(!parameters)
        return CONTENT_ERROR_CODES::CONTENT_PARAMETER_CAST;

    uniformBuffers = parameters->uniformBlocks;

    this->shaderType = parameters->type;
    std::string shaderSource = ReadSourceFromFile(filePath);

    if(shaderSource.empty())
        return CONTENT_ERROR_CODES::COULDNT_OPEN_CONTENT_FILE;

    if(!CompileFromSource(shaderSource))
        return CONTENT_ERROR_CODES::CREATE_FROM_MEMORY;

    return CONTENT_ERROR_CODES::NONE;
}

void GLShader::Unload(ContentManager* contentManager)
{
    if(shader != 0)
    {
        // If the draw binds are unloaded first glDetachShader will be called from there
        // and the shader program will be set to 0
        for(auto shaderProgram : shaderPrograms)
            if(shaderProgram->GetShaderProgram() != 0)
                glDetachShader(shaderProgram->GetShaderProgram(), shader);

        glDeleteShader(shader);

        shader = 0;
    }
}

CONTENT_ERROR_CODES GLShader::BeginHotReload(const char* filePath, ContentManager* contentManager)
{
    shaderSource = ReadSourceFromFile(filePath);

    if(shaderSource.empty())
        return CONTENT_ERROR_CODES::COULDNT_OPEN_CONTENT_FILE;

    return CONTENT_ERROR_CODES::NONE;
}

bool GLShader::ApplyHotReload()
{
    bool returnValue = CompileFromSource(shaderSource);

    shaderSource.resize(0);

    // TODO: Init uniform blocks here?

    return returnValue;
}

DiskContent* GLShader::CreateInstance() const
{
    return new GLShader(shaderType);
}

bool GLShader::CompileFromSource(const std::string& source)
{
    shader = glCreateShader((GLuint)shaderType);
    GLchar const* data = &source[0];
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

        Logger::LogLine(LOG_TYPE::FATAL, "Error when compiling shader at \"", this->GetPath(), "\": ", const_cast<const char*>(errorLog.get()));

        return false;
    }

    return true;
}

int GLShader::GetStaticVRAMUsage() const
{
    return 0;
}

int GLShader::GetDynamicVRAMUsage() const
{
    return 0;
}

int GLShader::GetRAMUsage() const
{
    return 0;
}

std::string GLShader::ReadSourceFromFile(const std::string& path)
{
    std::ifstream in(path, std::ios::ate);
    if(!in.is_open())
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't open file at \"", path, "\"");
        return "";
    }

    std::string shaderSource;
    shaderSource.resize(in.tellg(), 0);
    in.seekg(std::ios::beg);
    in.read(&shaderSource[0], shaderSource.size());

    return shaderSource;
}

void GLShader::InitUniformBlocks(GLuint shaderProgram) // TODO: Warn about unset blocks
{
    for(const auto block : uniformBuffers)
    {
        const GLuint blockIndex = glGetUniformBlockIndex(shaderProgram, block->GetName().c_str());

        if(blockIndex == (GLuint)-1)
        {
            Logger::LogLine(LOG_TYPE::WARNING, "No uniform block named \""
                                               + block->GetName()
                                               + "\" in shader \""
                                               + GetPath()
                                               + "\"");
            continue;
        }

        block->blockIndex = blockIndex;

        GLint numberOfUniforms;
        glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numberOfUniforms);

        if(block->GetNumberOfUniforms() != numberOfUniforms)
            Logger::LogLine(LOG_TYPE::WARNING
                        , "Number of uniforms in uniform block \""
                          + block->GetName()
                          + "\" isn't equal to the number uniforms in shader ("
                          + std::to_string(block->GetNumberOfUniforms())
                          + " != "
                          + std::to_string(numberOfUniforms)
                          + "). Whichever number is smallest will be used"); // TODO: Maybe delete them or something?

        numberOfUniforms = std::min(numberOfUniforms, block->GetNumberOfUniforms());

        std::vector<const GLchar*> uniformNames;

        int i = 0;
        for(const auto& uniform : block->uniforms)
        {
            if(i == numberOfUniforms)
                break;

            uniformNames.push_back(uniform.first.c_str()); // TODO: Is this smart?

            ++i;
        }

        std::unique_ptr<GLuint> indices(new GLuint[numberOfUniforms]);
        glGetUniformIndices(shaderProgram, numberOfUniforms, &uniformNames[0], indices.get());

        std::unique_ptr<GLint> offsets(new GLint[numberOfUniforms]);
        glGetActiveUniformsiv(shaderProgram, numberOfUniforms, indices.get(), GL_UNIFORM_OFFSET, offsets.get());

        GLint blockSize;
        glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);
        void* data = malloc(blockSize);
        std::memset(data, 0, blockSize);

        i = 0;
        for(const auto& name : uniformNames)
        {
            block->uniforms[name]->offset = offsets.get()[i];
            block->uniforms[name]->CopyValue(((GLubyte*)data) + offsets.get()[i]);

            ++i;
        }

        GLuint glBuffer;
        glGenBuffers(1, &glBuffer);

        block->bufferIndex = glBuffer;

        glBindBuffer(GL_UNIFORM_BUFFER, glBuffer);
        glBufferData(GL_UNIFORM_BUFFER, blockSize, data, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        free(data);
    }
}

void GLShader::BindUniformObjects()
{
    for(auto buffer : uniformBuffers)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer->blockIndex, buffer->bufferIndex);
    }
}

void GLShader::UnbindUniformObjects()
{
    for(auto buffer : uniformBuffers)
    {
        glBindBufferBase(GL_UNIFORM_BUFFER, buffer->blockIndex, 0);
    }
}
