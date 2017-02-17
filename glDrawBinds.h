#ifndef GLSHADERRESOURCEBINDS_H__
#define GLSHADERRESOURCEBINDS_H__

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>

#include "glEnums.h"
#include "glShader.h"
#include "glVertexBuffer.h"
#include "glIndexBuffer.h"
#include "glUniform.h"
#include "glInputLayout.h"
#include "contentManager.h"
#include "glUniformBuffer.h"
#include "glShaderStorageBuffer.h"

class GLVariable
{
public:
    GLVariable()
            : parent(nullptr)
              , uniform(nullptr)
              //, uniformBufferVariable(nullptr)
              , uniformBuffer(nullptr)
              , storageBuffer(nullptr)
    {}

    GLVariable(GLDrawBinds* parent, GLUniformBase* uniform)
            : parent(parent)
              , uniform(uniform)
              //, uniformBufferVariable(nullptr)
              , uniformBuffer(nullptr)
              , storageBuffer(nullptr)
    {}

    /*GLVariable(GLDrawBinds* parent, GLUniformBufferVariable* uniformBufferVariable)
            : parent(parent)
              , uniform(nullptr)
              , uniformBufferVariable(uniformBufferVariable)
              , uniformBuffer(nullptr)
    {}*/

    GLVariable(GLDrawBinds* parent, GLUniformBuffer* uniformBuffer)
            : parent(parent)
              , uniform(nullptr)
              //, uniformBufferVariable(nullptr)
              , uniformBuffer(uniformBuffer)
              , storageBuffer(nullptr)
    {}

    GLVariable(GLDrawBinds* parent, GLShaderStorageBuffer* storageBuffer)
            : parent(parent)
              , uniform(nullptr)
            //, uniformBufferVariable(nullptr)
              , uniformBuffer(nullptr)
              , storageBuffer(storageBuffer)
    {}

    template<typename T>
    void operator=(T value);

    template<typename T>
    void operator=(const std::vector<T>& values);

private:
    GLDrawBinds* parent;
    GLUniformBase* uniform;
    //GLUniformBufferVariable* uniformBufferVariable;
    GLUniformBuffer* uniformBuffer;
    GLShaderStorageBuffer* storageBuffer;
};

class GLDrawBinds
{
    friend class GLShader;
    friend class GLVariable;
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

        uniformBinds.insert(std::make_pair(name, std::unique_ptr<GLUniformBase>(new GLUniform<T>(data))));
        //uniformBinds[name] = new GLUniform<T>(data);
    }

    template<typename T>
    void AddUniform(const std::string& name, T data, int count)
    {
#ifndef NDEBUG
        if(uniformBinds.count(name) != 0)
            LogWithName(LOG_TYPE::DEBUG, "Adding uniform \"" + name + "\" multiple time to resource drawBinds");
#endif // NDEBUG

        uniformBinds.insert(std::make_pair(name, std::unique_ptr<GLUniformBase>(new GLUniformArray<T>(data))));
        //uniformBinds[name] = new GLUniformArray<T>(data, count);
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
    bool IsBound() const;

    GLShader* GetShader(GLEnums::SHADER_TYPE type, int index) const;
    int GetShaderTypeCount(GLEnums::SHADER_TYPE type) const;
    void DrawElements(GLEnums::DRAW_MODE drawMode = GLEnums::DRAW_MODE::TRIANGLES);
    void DrawElements(GLsizei count, GLEnums::DRAW_MODE drawMode = GLEnums::DRAW_MODE::TRIANGLES);
    void DrawElements(GLsizei count, GLsizei offset, GLEnums::DRAW_MODE drawMode = GLEnums::DRAW_MODE::TRIANGLES);
    void DrawElementsInstanced(int instances, GLEnums::DRAW_MODE drawMode = GLEnums::DRAW_MODE::TRIANGLES);

    GLVariable operator[](const std::string& name);
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

    GLuint currentBindingPoint;

    GLIndexBuffer* indexBuffer;
    std::vector<GLVertexBuffer*> vertexBuffers;
    std::vector<GLInputLayout> inputLayouts;
    std::vector<std::pair<GLEnums::SHADER_TYPE, GLShader*>> shaderBinds;
    std::map<std::string, std::unique_ptr<GLUniformBase>> uniformBinds;
    std::map<std::string, std::unique_ptr<GLUniformBuffer>> uniformBufferBinds;
    std::map<std::string, std::unique_ptr<GLShaderStorageBuffer>> storageBufferBinds;

    std::set<std::string> alreadyWarned;

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

    template<typename T>
    void UpdateUniform(GLUniformBase* uniform, T value)
    {
        uniform->operator=(value);

        if(IsBound())
            uniform->UploadData();
    }
    template<typename T>
    void UpdateUniformBufferVariable(GLUniformBufferVariable* variable, T value)
    {
        variable->operator=(value);
    }
    template<typename T>
    void UpdateUniformBuffer(GLUniformBuffer* buffer, const std::vector<T>& values)
    {
        buffer->SetData(values);
    }
    template<typename T>
    void UpdateUniformBuffer(GLUniformBuffer* buffer, const T& value)
    {
        buffer->SetData(value);
    }
    template<typename T>
    void UpdateStorageBuffer(GLShaderStorageBuffer* buffer, const T& value)
    {
        buffer->SetData(&value, sizeof(T));
    }

    void GetActiveStorageBlocks();
    void GetActiveUniformBlocks();
};

template<typename T>
void GLVariable::operator=(T value)
{
    static_assert(!std::is_pointer<T>::value, "Pointers aren't allowed, use std::vector instead");

    if(uniform)
        parent->UpdateUniform(uniform, value);
    //else if(uniformBufferVariable) // Both may be nullptr
    //    parent->UpdateUniformBufferVariable(uniformBufferVariable, value);
    else if(uniformBuffer)
        parent->UpdateUniformBuffer(uniformBuffer, value);
    else if(storageBuffer)
        parent->UpdateStorageBuffer(storageBuffer, value);
}

template<typename T>
void GLVariable::operator=(const std::vector<T>& values)
{
    if(uniformBuffer)
        parent->UpdateUniformBuffer(uniformBuffer, values);
}

#endif // GLSHADERRESOURCEBINDS_H__
