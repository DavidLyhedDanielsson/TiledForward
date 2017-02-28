#include <fstream>
#include <vector>
#include <sstream>
#include "glCPPShared.h"
#include "logger.h"
#include "contentManager.h"

GLCPPShared::GLCPPShared()
{}

GLCPPShared::~GLCPPShared()
{}

int GLCPPShared::GetStaticVRAMUsage() const
{
    return 0;
}

int GLCPPShared::GetDynamicVRAMUsage() const
{
    return 0;
}

int GLCPPShared::GetRAMUsage() const
{
    return 0;
}

CONTENT_ERROR_CODES GLCPPShared::Load(const char* filePath
                                      , ContentManager* contentManager
                                      , ContentParameters* contentParameters)
{
    auto parameters = TryCastTo<GLCPPSharedContentParameters>(contentParameters);
    if(parameters == nullptr)
        return CONTENT_ERROR_CODES::CONTENT_CREATION_PARAMETERS_CAST;

    this->contentManager = contentManager;
    this->inFile = filePath;

    std::ifstream in(inFile);
    if(!in.is_open())
        return CONTENT_ERROR_CODES::COULDNT_OPEN_CONTENT_FILE;
    in.close();

    this->variables = parameters->variables;

    std::string source = ReadAndParse(filePath);

    std::string outDir = std::string(filePath);
    outDir = outDir.substr(0, outDir.find_last_of("/\\"));

    std::string outFileName = std::string(filePath);
    outFileName = outFileName.substr(outFileName.find_last_of("/\\") + 1);
    outFileName = outFileName.substr(0, outFileName.find_last_of('.'));

    if(parameters->outPath.empty())
        outPath = outDir + "/" + outFileName + ".glsh";
    else
        outPath = parameters->outPath + "/" + outFileName + ".glsh";

    if(!WriteGLSHFile(source))
        return CONTENT_ERROR_CODES::COULDNT_OPEN_DEPENDENCY_FILE;

    return CONTENT_ERROR_CODES::NONE;
}

std::string GLCPPShared::ReadAndParse(const std::string& file)
{
    std::string source = ReadSourceFromFile(file);
    Parse(source);

    return source;
}

bool GLCPPShared::WriteGLSHFile(const std::string& data)
{
    std::ofstream out(outPath);
    if(!out.is_open())
        return false;

    out.write(data.c_str(), data.size());
    return true;
}

void GLCPPShared::Unload(ContentManager* contentManager)
{

}

void GLCPPShared::Parse(std::string& shaderSource)
{
    std::stringstream sstream(shaderSource);

    std::string line;
    auto lastG = sstream.tellg();
    while(std::getline(sstream, line))
    {
        std::string newData;

        if(line.compare(0, 7, "include") == 0)
            newData = ParseInclude(line);
        else if(line.compare(0, 6, "cconst") == 0)
            newData = ParseVariable(line);

        if(!newData.empty())
        {
            shaderSource.replace(lastG, line.length(), newData);

            sstream.str(shaderSource);
            sstream.seekg((int)lastG);
        }

        lastG = (int)sstream.tellg();
    }
}

std::string GLCPPShared::ParseVariable(const std::string& line)
{
    // Skip "cconst "
    std::string name = line.substr(7);

    if(name.back() == ';')
        name.pop_back();

    std::string outValue = "0";

    if(variables.count(name) == 0)
        Logger::LogLine(LOG_TYPE::WARNING, "Couldn't find value for \"" + name + "\"; it will be set to 0");
    else
        outValue = variables.at(name);

    return "#define " + name + " " + outValue;
}

std::string GLCPPShared::ParseInclude(const std::string& line)
{
    // Skip to until name
    std::string name = line.substr(line.find(' ') + 2);
    // Remove "
    name.pop_back();

    std::string thisPath = GetPath();
    thisPath = thisPath.substr(0, thisPath.find_last_of("/\\") + 1);

    std::string includeFileSource = ReadSourceFromFile(thisPath + name);
    if(includeFileSource.empty())
    {
        Logger::LogLine(LOG_TYPE::FATAL, "Couldn't find include file at \"" + thisPath + name + "\"");
        return "";
    }

    Parse(includeFileSource);
    return includeFileSource;
}

std::string GLCPPShared::ReadSourceFromFile(const std::string& path)
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

bool GLCPPShared::SetValue(const std::string& name, const std::string& newValue)
{
    if(variables.count(name) == 0)
        return false;

    variables.at(name) = newValue;

    return true;
}

void GLCPPShared::WriteValues()
{
    WriteGLSHFile(ReadAndParse(inFile));

    std::vector<DiskContent*> diskContentUsages;

    for(GLShader* usage : usages)
        diskContentUsages.push_back(usage);

    contentManager->ForceHotReload(diskContentUsages);
}

void GLCPPShared::AddUsage(GLShader* shader)
{
    usages.push_back(shader);
}
