#ifndef GLSHADERRESOURCEBINDS_H__
#define GLSHADERRESOURCEBINDS_H__

#include <string>
#include <vector>
#include <memory>

#include "glEnums.h"
#include "glShader.h"
#include "glShaderProgram.h"
#include "glVertexBuffer.h"
#include "glIndexBuffer.h"

class GLDrawBinds
{
public:
    GLDrawBinds();
    ~GLDrawBinds();

    bool Init();

    void AddShader(GLEnums::SHADER_TYPE type, const std::string& shaderPath);
    template<typename... T>
    void AddShaders(GLEnums::SHADER_TYPE type, const std::string& shaderPath, T... rest)
    {
        AddShader(type, shaderPath);

        AddShaders(rest...);
    }

    void AddUniform(const std::string& name, void* data);
    void AddUniform(const std::string& name, void* data, size_t dataSize);
    template<typename... T>
    void AddUniforms(const std::string& name, void* data, T... rest)
    {
        AddUniform(name, data);

        AddUniforms(rest...);
    }
    template<typename... T>
    void AddUniforms(const std::string& name, void* data, size_t dataSize, T... rest)
    {
        AddUniform(name, data, dataSize);

        AddUniforms(rest...);
    }

    template<typename First, typename... Rest>
    void AddBuffers(First buffer, Rest... rest)
    {
        AddBuffer(buffer);
        AddBuffers(rest...);
    }

    void Bind();
    void Unbind();

    GLShader* GetShader(GLEnums::SHADER_TYPE type, int index) const;
    int GetShaderTypeCount(GLEnums::SHADER_TYPE type) const;
    void DrawElements();
protected:
private:
    struct Attrib
    {
        std::unique_ptr<GLchar> name;
        GLint size;
        GLenum type;

        Attrib(std::unique_ptr<GLchar>&& name, GLint size, GLenum type)
                : name(std::move(name)), size(size), type(type)
        {}
    };

    template<typename T>
    void AddBuffer(T buffer)
    { }

    bool bound;

    GLuint shaderProgram;
    GLuint vao;

    GLIndexBuffer* indexBuffer;
    std::vector<GLVertexBuffer*> vertexBuffers;
    std::vector<std::pair<GLEnums::SHADER_TYPE, GLShader*>> shaderBinds;
    std::vector<std::pair<std::string, std::pair<void*, bool>>> uniformBinds;

    // Recursive template termination
    void AddShaders()
    {}

    // Recursive template termination
    void AddUniforms()
    {}

    // Recursive template termination
    void AddBuffers()
    {}

    bool VertexOrComputeShaderAvailable() const;
    std::vector<Attrib> GetActiveAttribs() const;
    bool CreateShaderProgram();

    int GetNumberOfFloats(GLenum type) const;
    bool CheckRequirements() const;
};

template<>
inline void GLDrawBinds::AddBuffer(GLVertexBuffer* vertexBuffer)
{
    vertexBuffers.push_back(vertexBuffer);
}

template<>
inline void GLDrawBinds::AddBuffer(GLIndexBuffer* indexBuffer)
{
#ifndef NDEBUG
    if(this->indexBuffer != nullptr)
        Logger::LogLine(LOG_TYPE::WARNING, "Overwriting index buffer");
#endif // NDEBUG

    this->indexBuffer = indexBuffer;
}

#endif // GLSHADERRESOURCEBINDS_H__
