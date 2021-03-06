#ifndef GLSHADER_H__
#define GLSHADER_H__

#include "../content/content.h"

#include <string>
#include <vector>
#include <memory>

#include "glEnums.h"
#include "../content/contentManager.h"

class GLUniformBuffer;

class GLDrawBinds;

class GLShader
        : public DiskContent
{
    friend class GLDrawBinds;
public:
    GLShader();
    virtual ~GLShader();

    GLEnums::SHADER_TYPE GetShaderType() const;
    GLuint GetShader() const;

    bool CreateDefaultContent(const char* filePath, ContentManager* contentManager) override;

    virtual int GetStaticVRAMUsage() const;
    virtual int GetDynamicVRAMUsage() const;
    virtual int GetRAMUsage() const;

protected:
    CONTENT_ERROR_CODES Load(const char* filePath
                             , ContentManager* contentManager
                             , ContentParameters* contentParameters) override;
    void Unload(ContentManager* contentManager) override;

    CONTENT_ERROR_CODES BeginHotReload(const char* filePath, ContentManager* contentManager) override;
    bool ApplyHotReload() override;
    bool Apply(Content* content) override;

    DiskContent* CreateInstance() const override;

private:
    GLShader(GLEnums::SHADER_TYPE type);

    GLEnums::SHADER_TYPE shaderType;
    GLuint shader;

    std::string shaderSource; //For hot reloading
    std::vector<GLDrawBinds*> shaderPrograms; //For hot reloading

    std::vector<std::pair<std::string, std::string>> variables;

    bool CompileFromSource(const std::string& source);
    std::string ReadSourceFromFile(const std::string& path);

    void Parse(std::string& shaderSource, const std::vector<std::pair<std::string, std::string>>& variables, ContentManager* contentManager);
    //void Parse(std::string& shaderSource, ContentManager* contentManager);
    std::string ParseVariable(const std::string& line, const std::vector<std::pair<std::string, std::string>>& variables);
    std::string ParseInclude(const std::string& line, const std::vector<std::pair<std::string, std::string>>& variables, ContentManager* contentManager);
};

#endif // GLSHADER_H__
