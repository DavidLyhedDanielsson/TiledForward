#ifndef GLUNIFORMBLOCK_H__
#define GLUNIFORMBLOCK_H__

#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <memory>

#include "logger.h"
#include "glEnums.h"

class GLUniformBuffer;

struct GLUniformBufferVariable
{
    friend class GLShader;
    friend class GLUniformBuffer;

    template<typename T>
    GLUniformBufferVariable(GLUniformBuffer& parent, T* initialValue, int elementCount, int offset)
            : elementSize(sizeof(T))
              , elementCount(elementCount)
              , parent(parent)
              , offset(offset)
    {
#ifndef NDEBUG
        verifier.reset(new Verifier<T>());
#endif // NDEBUG

        CopyDataToParent(initialValue);
    }

    template<typename T>
    GLUniformBufferVariable(GLUniformBuffer& parent, T initialValue, int offset)
            : GLUniformBufferVariable(parent, &initialValue, 1, offset)
    { }

    ~GLUniformBufferVariable() = default;

    template<typename T>
    void operator=(T newData)
    {
#ifndef NDEBUG
        verifier->Verify(newData);
#endif // NDEBUG

        CopyDataToParent(&newData);
    }

    template<typename T>
    void operator=(T* newData)
    {
#ifndef NDEBUG
        verifier->Verify(newData);
#endif // NDEBUG

        CopyDataToParent(newData);
    }

    int GetElementSize() const
    {
        return elementSize;
    }

    int GetElementCount() const
    {
        return elementCount;
    }

    int GetSize() const
    {
        return elementSize * elementCount;
    }

    int GetOffset() const
    {
        return offset;
    }

protected:
    int elementSize;
    int elementCount;

    int offset;

private:
    GLUniformBuffer& parent;

    void CopyDataToParent(void* data);

#ifndef NDEBUG
    struct VerifierBase
    {
        template<typename T>
        void Verify(T value)
        {
            if(dynamic_cast<Verifier<T>*>(this) == nullptr)
                throw std::runtime_error("Couldn't cast type to T");
        }

    protected:
        VerifierBase() = default;
        VerifierBase(const VerifierBase&) = default;
        VerifierBase(VerifierBase&&) = default;
        virtual ~VerifierBase() = default;
    };

    template<typename T>
    struct Verifier : public VerifierBase
    {
        Verifier() = default;
    };

    std::shared_ptr<VerifierBase> verifier;
#endif // NDEBUG
};

class GLUniformBuffer
{
public:
    friend class GLShader;
    friend class GLUniformBufferVariable;

    GLUniformBuffer(const std::string& name, GLuint shaderProgram, GLuint blockIndex);
    ~GLUniformBuffer();

    bool Init();
    void Bind();
    void Unbind();

    void SetData(void* data, GLsizei dataSize);

    GLUniformBufferVariable& operator[](const std::string& name)
    {
    #ifndef NDEBUG
        if(uniforms.count(name) == 0)
            throw std::runtime_error("Trying to get non-existent variable \"" + name + "\" in block \"" + this->name + "\"");
    #endif // NDEBUG

        return uniforms.at(name);
    };

    bool VariableExists(const std::string& variable) const
    {
        return uniforms.count(variable) != 0;
    }

    int GetSize() const
    {
        int totalSize = 0;

        for(const auto& pair : uniforms)
            totalSize += pair.second.GetSize();

        return totalSize;
    }

    int GetNumberOfUniforms() const
    {
        return uniforms.size();
    }

    const std::string& GetName() const
    {
        return name;
    }

private:
    class Uniform
    {
    public:
        Uniform(const std::string& name)
                : name(name)
        {}

        Uniform(const std::string& name, GLEnums::UNIFORM_TYPE type, int count)
                : name(name), type(type), count(count)
        {}

        Uniform(const std::string& name, GLenum type, int count)
                : name(name), type((GLEnums::UNIFORM_TYPE)type), count(count)
        {}

        std::string name;
        GLEnums::UNIFORM_TYPE type;
        // These are placed in a set, so count needs to be mutable (name is compared anyway)
        mutable int count; // If array, count > 1

        bool operator<(const Uniform& rhs) const
        {
            return name < rhs.name;
        }

        bool operator<(const std::string& rhs) const
        {
            return name < rhs;
        }
    };

    struct uniquePtrFree
    {
        void operator()(void* ptr)
        {
            free(ptr);
        }
    };

    GLuint shaderProgram;
    GLuint blockIndex;
    GLuint bufferIndex;
    std::string name;

    GLint size;
    std::unique_ptr<void, uniquePtrFree> data;
    bool modifiedSinceCopy;

    std::map<std::string, GLUniformBufferVariable> uniforms; // Make sure offsets are consecutive
};

#endif // GLUNIFORMBLOCK_H__
