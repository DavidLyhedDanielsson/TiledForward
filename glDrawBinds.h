#ifndef GLSHADERRESOURCEBINDS_H__
#define GLSHADERRESOURCEBINDS_H__

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "glEnums.h"
#include "glShader.h"
#include "glVertexBuffer.h"
#include "glIndexBuffer.h"
#include "glUniform.h"
#include "glInputLayout.h"
#include "contentManager.h"

class GLDrawBinds
{
    friend class GLShader;
public:
    GLDrawBinds();
    ~GLDrawBinds();

    bool Init();

    void AddShader(ContentManager& contentManager, GLEnums::SHADER_TYPE type, const std::string& shaderPath);
    template<typename... T>
    void AddShaders(ContentManager& contentManager, GLEnums::SHADER_TYPE type, const std::string& shaderPath, T... rest)
    {
        AddShader(contentManager, type, shaderPath);

        AddShaders(contentManager, rest...);
    }

    template<typename T>
    void AddUniform(const std::string& name, T data)
    {
#ifndef NDEBUG
        if(uniformBinds.count(name) != 0)
            LogWithName(LOG_TYPE::DEBUG, "Adding uniform \"" + name + "\" multiple time to resource binds");
#endif // NDEBUG

        uniformBinds[name] = new GLUniform<T>(data);
    }

    template<typename T>
    void AddUniform(const std::string& name, T data, int count)
    {
#ifndef NDEBUG
        if(uniformBinds.count(name) != 0)
            LogWithName(LOG_TYPE::DEBUG, "Adding uniform \"" + name + "\" multiple time to resource binds");
#endif // NDEBUG

        uniformBinds[name] = new GLUniformArray<T>(data, count);
    }

    template<typename T, typename... Rest>
    void AddUniforms(const std::string& name, T data, Rest... rest)
    {
        AddUniform(name, data);
        AddUniforms(rest...);
    }
    template<typename T, typename... Rest>
    void AddUniforms(const std::string& name, T* data, Rest... rest)
    {
        AddUniform(name, data);
        AddUniforms(rest...);
    }

    template<typename First, typename... Rest>
    void AddBuffers(First buffer, Rest... rest)
    {
        AddBuffer(buffer);
        AddBuffers(rest...);
    }

    template<typename VertexBuffer, typename... Rest>
    void AddBuffers(VertexBuffer buffer, const GLInputLayout& inputLayout, Rest... rest)
    {
        AddBuffer(buffer, inputLayout);
        AddBuffers(rest...);
    }

    void Bind();
    void Unbind();

    GLShader* GetShader(GLEnums::SHADER_TYPE type, int index) const;
    int GetShaderTypeCount(GLEnums::SHADER_TYPE type) const;
    void DrawElements();
    void DrawElementsInstanced(int instances);

    GLUniformBase& operator[](const std::string& name)
    {
        return *uniformBinds[name];
    }

    void CompileProgram();

protected:
private:
    // Also used for uniforms
    struct Attrib
    {
        std::string name;
        GLint size;
        GLenum type;

        Attrib(std::string&& name, GLint size, GLenum type)
                : name(std::move(name)), size(size), type(type)
        {}
    };

    bool bound;

    GLuint shaderProgram;
    GLuint vao;

    GLIndexBuffer* indexBuffer;
    std::vector<GLVertexBuffer*> vertexBuffers;
    std::vector<GLInputLayout> inputLayouts;
    std::vector<std::pair<GLEnums::SHADER_TYPE, GLShader*>> shaderBinds;
    std::map<std::string, GLUniformBase*> uniformBinds;

    // Recursive template termination
    void AddShaders()
    {}

    void AddUniforms()
    {}

    void AddBuffers()
    {}

    void AddShaders(ContentManager& contentManager)
    { }

    void AddBuffer(GLVertexBuffer* vertexBuffer);
    void AddBuffer(GLVertexBuffer* vertexBuffer, GLInputLayout inputLayout);
    void AddBuffer(GLIndexBuffer* indexBuffer);

    std::vector<Attrib> GetActiveAttribs() const;
    std::vector<Attrib> GetActiveUniforms() const;
    bool CreateShaderProgram();
    void BindUniforms();

    int GetNumberOfFloats(GLenum type) const;

    bool CheckUniforms(const std::vector<Attrib>& activeUniforms);
    bool CheckRequirements() const;

    std::string ToString() const;

    void LogWithName(LOG_TYPE logType, const std::string& message) const;

    void RelinkShaders();
    GLuint GetShaderProgram() const;
};

#endif // GLSHADERRESOURCEBINDS_H__
