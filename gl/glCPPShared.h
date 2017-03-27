#ifndef GLCPPSHARED_H__
#define GLCPPSHARED_H__

#include <map>
#include "../content/content.h"
#include "glShader.h"

struct GLCPPSharedContentParameters
    : public ContentParameters
{
    std::map<std::string, std::string> variables;
    std::string outPath;
};

class GLCPPShared
    : public MemoryContent
{
public:
    GLCPPShared();
    ~GLCPPShared();

    int GetStaticVRAMUsage() const override;
    int GetDynamicVRAMUsage() const override;
    int GetRAMUsage() const override;

    bool SetValue(const std::string& name, const std::string& newValue);
    void WriteValues();

    void AddUsage(GLShader* shader);

protected:
    CONTENT_ERROR_CODES Load(const char* filePath
                             , ContentManager* contentManager
                             , ContentParameters* contentParameters) override;
    void Unload(ContentManager* contentManager) override;
protected:
private:
    std::map<std::string, std::string> variables;
    std::string inFile;
    std::string outPath;

    ContentManager* contentManager;

    std::vector<GLShader*> usages;

    std::string ReadAndParse(const std::string& file);
    std::string ReadSourceFromFile(const std::string& path);
    bool WriteGLSHFile(const std::string& data);

    void Parse(std::string& shaderSource);
    std::string ParseVariable(const std::string& line);
    std::string ParseInclude(const std::string& line);
};

#endif // GLCPPSHARED_H__
