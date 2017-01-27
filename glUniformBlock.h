#ifndef GLUNIFORMBLOCK_H__
#define GLUNIFORMBLOCK_H__

#include <string>
#include <cstring>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <memory>

#include "logger.h"

template<typename T>
struct GLUniformBlockVariable;

struct GLUniformBlockVariableBase
{
    friend class GLShader;
    friend class GLUniformBlock;

    GLUniformBlockVariableBase();
    virtual ~GLUniformBlockVariableBase() = default;

    template<typename T>
    void operator=(T value)
    {
        GLUniformBlockVariable<T>* thisAsDerived = dynamic_cast<GLUniformBlockVariable<T>*>(this);

        if(thisAsDerived != nullptr)
            thisAsDerived->SetValue(value);
        else
            throw std::runtime_error("Couldn't cast type to T");
    }

    virtual void CopyValue(void* destination) = 0;

    int GetSize() const
    {
        return size;
    }

    int GetOffset() const
    {
        return offset;
    }

protected:
    int size;
    int offset;

    bool modified;
};

template<typename T>
struct GLUniformBlockVariable
        : public GLUniformBlockVariableBase
{
    friend class GLUniformBlock;

    GLUniformBlockVariable(T value)
            : value(value)
    {
        this->size = sizeof(T);
    }

    void SetValue(T value)
    {
        modified = true;

        this->value = value;
    }

    void CopyValue(void* destination)
    {
        std::memcpy((char*)destination + offset, &value, sizeof(T));
    }

private:
    T value;
};

class GLUniformBlock
{
public:
    friend class GLShader;

    GLUniformBlock(const std::string& name);
    ~GLUniformBlock();

    template<typename T>
    void AddVariable(const std::string& variable, T value)
    {
#ifndef NDEBUG
        for(const auto& pair : uniforms)
        {
            if(pair.first == variable)
            {
                if(pair.second->GetSize() == sizeof(T))
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
                                                     + "(old = ", pair.second->GetSize(), ", new = ", sizeof(T));

                return;
            }
        }
#endif // NDEBUG

        uniforms.insert(std::make_pair(variable, new GLUniformBlockVariable<T>(value)));
    }

    bool Init(GLuint shaderProgram);
    void UploadDataIfNeeded();

    GLUniformBlockVariableBase& operator[](const std::string& name)
    {
#ifndef NDEBUG
        if(uniforms.count(name) == 0)
            throw std::runtime_error("Trying to get non-existent variable \"" + name + "\" in block \"" + this->name + "\"");
#endif // NDEBUG

        return *uniforms[name];
    };

    bool VariableExists(const std::string& variable) const
    {
        return uniforms.count(variable) != 0;
    }

    int GetSize() const
    {
        int totalSize = 0;

        for(const auto& pair : uniforms)
            totalSize += pair.second->GetSize();

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

    std::map<std::string, GLUniformBlockVariableBase*> uniforms; // Make sure offsets are consecutive
};

#endif // GLUNIFORMBLOCK_H__
