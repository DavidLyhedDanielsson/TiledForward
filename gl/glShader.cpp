#include <fstream>
#include <memory>
#include "glShader.h"
#include "../logger.h"
#include "../content/shaderContentParameters.h"

#include "glDrawBinds.h"
#include "glUniformBuffer.h"
#include "glCPPShared.h"

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
        case GLEnums::SHADER_TYPE::FRAGMENT:
        case GLEnums::SHADER_TYPE::TESS_CONTROL:
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
    this->shaderSource = std::move(shader->shaderSource);

    // TODO: Move?
    for(auto shaderProgram : shaderPrograms)
        glAttachShader(shaderProgram->GetShaderProgram(), this->shader);

    for(auto shaderProgram : shaderPrograms)
        shaderProgram->RelinkShaders();

    //Parse(shaderSource, variables, nullptr); // TODO: Support this?
    shaderSource.resize(0);

    return true;
}

CONTENT_ERROR_CODES GLShader::Load(const char* filePath
                                   , ContentManager* contentManager
                                   , ContentParameters* contentParameters)
{
    ShaderContentParameters* parameters = TryCastTo<ShaderContentParameters>(contentParameters);
    if(!parameters)
        return CONTENT_ERROR_CODES::CONTENT_PARAMETER_CAST;

    this->shaderType = parameters->type;
    this->variables = parameters->variables;

    std::string shaderSource = ReadSourceFromFile(filePath);

    if(shaderSource.empty())
        return CONTENT_ERROR_CODES::COULDNT_OPEN_CONTENT_FILE;

    Parse(shaderSource, parameters->variables, contentManager);

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

    Parse(shaderSource, variables, nullptr);

    if(shaderSource.empty())
        return CONTENT_ERROR_CODES::COULDNT_OPEN_CONTENT_FILE;

    return CONTENT_ERROR_CODES::NONE;
}

bool GLShader::ApplyHotReload()
{
    //Parse(shaderSource, variables, nullptr);
    bool returnValue = CompileFromSource(shaderSource);

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

        bool trimToLine = false;

        std::string errorLogString(errorLog.get());
        auto beginIndex = errorLogString.find_first_not_of("0123456789");
        auto endIndex = beginIndex;
        if(beginIndex != std::string::npos
            && errorLogString[beginIndex] == '(')
        {
            endIndex = errorLogString.find_first_not_of("0123456789", beginIndex + 1);

            if(errorLogString[endIndex] == ')')
                trimToLine = true;
        }

        if(!trimToLine)
            Logger::LogLine(LOG_TYPE::FATAL, "Error when compiling shader at \"", this->GetPath(), "\": ", const_cast<const char*>(errorLog.get()), "\nShader source:\n" + source);
        else
        {
            auto lineIndex = std::stoi(errorLogString.substr(beginIndex + 1, endIndex - beginIndex - 1));

            std::stringstream sstream(source);
            std::string line;
            int i = 0;
            while(std::getline(sstream, line))
            {
                ++i;

                if(i == lineIndex)
                    break;
            }

            Logger::LogLine(LOG_TYPE::FATAL, "Error when compiling shader at \"", this->GetPath(), "\": ", const_cast<const char*>(errorLog.get()), "\nShader source line:\n" + line);
        }

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

void GLShader::Parse(std::string& shaderSource
                     , const std::vector<std::pair<std::string, std::string>>& variables
                     , ContentManager* contentManager)
{
    std::stringstream sstream(shaderSource);

    std::string line;
    auto lastG = sstream.tellg();
    while(std::getline(sstream, line))
    {
        std::string newData;

        if(line.compare(0, 6, "cconst") == 0)
        {
            newData = ParseVariable(line, variables);

            if(!newData.empty())
            {
                shaderSource.replace(lastG, line.length(), newData);

                sstream.str(shaderSource);
                sstream.seekg((int)lastG + (int)newData.length() + 1);
            }
        }
        else if(line.compare(0, 8, "#include") == 0)
        {
            newData = ParseInclude(line, variables, contentManager);

            if(!newData.empty())
            {
                shaderSource.replace(lastG, line.length(), newData);

                sstream.str(shaderSource);
                sstream.seekg((int)lastG);
            }
        }

        lastG = (int)sstream.tellg();
    }
}

/*void GLShader::Parse(std::string& shaderSource, ContentManager* contentManager)
{
    std::stringstream sstream(shaderSource);

    std::string line;
    auto lastG = sstream.tellg();
    while(std::getline(sstream, line))
    {
        std::string newData;

        if(line.compare(0, 8, "#include") == 0)
            newData = ParseInclude(line, contentManager);

        if(!newData.empty())
        {
            shaderSource.replace(lastG, line.length(), newData);

            sstream.str(shaderSource);
            sstream.seekg((int)lastG);
        }

        lastG = (int)sstream.tellg();
    }
}*/

std::string GLShader::ParseVariable(const std::string& line, const std::vector<std::pair<std::string, std::string>>& variables)
{
    // Skip "cconst "
    std::string name = line.substr(7);
    // Remove ;
    name.pop_back();

    auto iter = variables.begin();
    for(; iter != variables.end(); ++iter)
    {
        if(iter->first == name)
            break;
    }

    if(iter == variables.end())
    {
        Logger::LogLine(LOG_TYPE::WARNING, std::string("Error parsing shader variable in \"")
                                           + GetPath()
                                           + "\", no variable named \""
                                           + name
                                           + "\" found");

        return "";
    }

    return "#define " + iter->first + " " + iter->second;
}

std::string GLShader::ParseInclude(const std::string& line, const std::vector<std::pair<std::string, std::string>>& variables, ContentManager* contentManager)
{
    // Skip to until name
    std::string name = line.substr(line.find(' ') + 2);
    // Remove "
    name.pop_back();

    std::string thisPath = GetPath();
    thisPath = thisPath.substr(0, thisPath.find_last_of("/\\") + 1);

    if(contentManager != nullptr)
    {
        std::string headerName = name.substr(0, name.find_last_of('.') + 1) + "h";

        auto shared = contentManager->GetLoadedContent<GLCPPShared>(thisPath + headerName);
        if(shared != nullptr)
            shared->AddUsage(this);
    }

    std::string includeFileSource = ReadSourceFromFile(thisPath + name);
    if(includeFileSource.empty())
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't find include file at \"" + thisPath + name + "\"");
        return "";
    }

    Parse(includeFileSource, variables, contentManager);
    return includeFileSource;
}
