#ifndef GLUNIFORMBLOCK_H__
#define GLUNIFORMBLOCK_H__

#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <memory>

#include "logger.h"

class GLUniformBlock;

struct GLUniformBlockVariable
{
    friend class GLShader;
    friend class GLUniformBlock;

    template<typename T>
    GLUniformBlockVariable(GLUniformBlock& parent, T* initialValue, int elementCount, int offset)
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
    GLUniformBlockVariable(GLUniformBlock& parent, T initialValue, int offset)
            : GLUniformBlockVariable(parent, &initialValue, 1, offset)
    { }

    ~GLUniformBlockVariable() = default;

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
    GLUniformBlock& parent;

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

class GLUniformBlock
{
public:
    friend class GLShader;
    friend class GLUniformBlockVariable;

    template<typename T>
    GLUniformBlock(const std::string& name, const std::string& variable, T* value, int count)
    {
        int currentSize = 0;

        Init(name, currentSize, variable, value, count);
    };

    template<typename T, typename... Rest>
    GLUniformBlock(const std::string& name, const std::string& variable, T* value, int count, Rest... rest)
    {
        int currentSize = 0;

        Init(name, currentSize, variable, value, count, rest...);
    };

    template<typename T, typename... Rest>
    GLUniformBlock(const std::string& name, const std::string& variable, T value, Rest... rest)
    {
        int currentSize = 0;

        Init(name, currentSize, variable, &value, 1, rest...);
    };
    ~GLUniformBlock();

    template<typename T>
    void AddVariable(const std::string& variable, T value, int offset)
    {
#ifndef NDEBUG
        for(const auto& pair : uniforms)
        {
            if(pair.first == variable)
            {
                if(pair.second.GetSize() == sizeof(T))
                    Logger::LogLine(LOG_TYPE::DEBUG, "Trying to add variable \""
                                                     + variable
                                                     + "\" to uniform block \""
                                                     + this->name
                                                     + "\" multiple times");
                else
                    Logger::LogLine(LOG_TYPE::DEBUG, "Trying to add variable \""
                                                     + variable
                                                     + "\" to uniform block \""
                                                     + this->name
                                                     + "\" multiple times with different sizes "
                                                     + "(old = ", pair.second.GetSize(), ", new = ", sizeof(T));

                return;
            }
        }
#endif // NDEBUG

        uniforms.emplace(variable, GLUniformBlockVariable(*this, value, offset));
    }

    template<typename T>
    void AddVariable(const std::string& variable, T* value, int count, int offset)
    {
#ifndef NDEBUG
        for(const auto& pair : uniforms)
        {
            if(pair.first == variable)
            {
                if(pair.second.GetSize() == sizeof(T))
                    Logger::LogLine(LOG_TYPE::DEBUG, "Trying to add variable \""
                                                     + variable
                                                     + "\" to uniform block \""
                                                     + this->name
                                                     + "\" multiple times");
                else
                    Logger::LogLine(LOG_TYPE::DEBUG, "Trying to add variable \""
                                                     + variable
                                                     + "\" to uniform block \""
                                                     + this->name
                                                     + "\" multiple times with different sizes "
                                                     + "(old = ", pair.second.GetSize(), ", new = ", sizeof(T));

                return;
            }
        }
#endif // NDEBUG

        uniforms.emplace(variable, GLUniformBlockVariable(*this, value, count, offset));
    }

    bool Init(GLuint shaderProgram);
    void UploadDataIfNeeded();

    GLUniformBlockVariable& operator[](const std::string& name)
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
    void Init(const std::string& name, int size);

    template<typename T>
    void Init(const std::string& name, int& currentSize, const std::string& variable, T* value, int count)
    {
        int offset = currentSize;

        currentSize += sizeof(T) * count;

        Init(name, currentSize);

        AddVariable(variable, value, count, offset);
    };

    template<typename T, typename... Rest>
    void Init(const std::string& name, int& currentSize, const std::string& variable, T* value, int count, Rest... rest)
    {
        int offset = currentSize;

        currentSize += sizeof(T) * count;

        Init(name, currentSize, rest...);

        AddVariable(variable, value, count, offset);
    };

    template<typename T>
    void Init(const std::string& name, int& currentSize, const std::string& variable, T value)
    {
        Init(name, currentSize, variable, &value, 1);
    };

    template<typename T, typename... Rest>
    void Init(const std::string& name, int& currentSize, const std::string& variable, T value, Rest... rest)
    {
        Init(name, currentSize, variable, &value, 1, rest...);
    };

    struct uniquePtrFree
    {
        void operator()(void* ptr)
        {
            free(ptr);
        }
    };

    GLuint blockIndex;
    GLuint bufferIndex;
    std::string name;

    GLint size;
    std::unique_ptr<void, uniquePtrFree> data;
    bool modifiedSinceCopy;

    std::map<std::string, GLUniformBlockVariable> uniforms; // Make sure offsets are consecutive
};

#endif // GLUNIFORMBLOCK_H__
