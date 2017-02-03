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
#include "glUniformBuffer.h"

class GLDrawBinds
{
    friend class GLShader;
public:
    GLDrawBinds();
    ~GLDrawBinds();

    bool Init();

    bool AddShader(ContentManager& contentManager, GLEnums::SHADER_TYPE type, const std::string& shaderPath);
    template<typename... T>
    bool AddShaders(ContentManager& contentManager, GLEnums::SHADER_TYPE type, const std::string& shaderPath, T... rest)
    {
        if(AddShader(contentManager, type, shaderPath))
            return AddShaders(contentManager, rest...);
        else
            return false;
    }

    template<typename T>
    void AddUniform(const std::string& name, T data)
    {
#ifndef NDEBUG
        if(uniformBinds.count(name) != 0)
            LogWithName(LOG_TYPE::DEBUG, "Adding uniform \"" + name + "\" multiple time to resource drawBinds");
#endif // NDEBUG

        uniformBinds[name] = new GLUniform<T>(data);
    }

    template<typename T>
    void AddUniform(const std::string& name, T data, int count)
    {
#ifndef NDEBUG
        if(uniformBinds.count(name) != 0)
            LogWithName(LOG_TYPE::DEBUG, "Adding uniform \"" + name + "\" multiple time to resource drawBinds");
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
    void DrawElements(GLEnums::DRAW_MODE drawMode = GLEnums::DRAW_MODE::TRIANGLES);
    void DrawElements(GLsizei count, GLEnums::DRAW_MODE drawMode = GLEnums::DRAW_MODE::TRIANGLES);
    void DrawElements(GLsizei count, GLsizei offset, GLEnums::DRAW_MODE drawMode = GLEnums::DRAW_MODE::TRIANGLES);
    void DrawElementsInstanced(int instances, GLEnums::DRAW_MODE drawMode = GLEnums::DRAW_MODE::TRIANGLES);

    GLUniformBuffer* GetUniformBuffer(const std::string& name);

    GLUniformBase* operator[](const std::string& name)
    {
#ifndef NDEBUG
        if(uniformBinds.count(name) == 0)
        {
            Logger::LogLine(LOG_TYPE::DEBUG, "Trying to get uniform \"" + name +  "\" which doesn't exist! Did you forget to call AddUniform?");
            return nullptr;
        }
#endif // NDEBUG

        return uniformBinds[name];
    }

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
    std::map<std::string, std::unique_ptr<GLUniformBuffer>> uniformBufferBinds;

    // Recursive template termination
    void AddShaders()
    {}

    void AddUniforms()
    {}

    void AddBuffers()
    {}

    bool AddShaders(ContentManager& contentManager)
    {
        return true;
    }

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
