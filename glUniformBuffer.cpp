#include "glUniformBuffer.h"
#include "glEnums.h"
#include "glHelpers.h"

#include <algorithm>
#include <set>

GLUniformBuffer::GLUniformBuffer(const std::string& name, GLuint shaderProgram, GLuint blockIndex, GLuint bindingPoint)
        : name(name)
          , size(0)
          , shaderProgram(shaderProgram)
          , blockIndex(blockIndex)
          , bindingPoint(bindingPoint)
{ }

GLUniformBuffer::~GLUniformBuffer()
{ }

void GLUniformBuffer::Bind()
{
    if(modifiedSinceCopy)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, bufferIndex);
        //glBufferSubData(GL_UNIFORM_BUFFER, blockIndex, size, data.get());
        GLvoid* mappedMemory = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(mappedMemory, data.get(), size);
        glUnmapBuffer(GL_UNIFORM_BUFFER);

        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        modifiedSinceCopy = false;
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, bufferIndex);
}

void GLUniformBuffer::Unbind()
{
    glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, 0);
}

bool GLUniformBuffer::Init()
{
    //this->size = 0;
    //this->data.reset(nullptr);
    //this->modifiedSinceCopy = true;

    // Get number of active uniforms and their indicies
    GLint numberOfUniformsInBlock;
    glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numberOfUniformsInBlock);

    std::unique_ptr<GLint> activeUniformIndicies(new GLint[numberOfUniformsInBlock]);
    glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, activeUniformIndicies.get());

    // Get name of all active uniforms
    GLint maxUniformNameLength;
    glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

    // Lots of commented code can be used for array index and stuff!
    std::vector<const GLchar*> uniformNames;

    // Add all active uniforms that are inside of this block to uniformsInBlock
    std::set<Uniform> uniformsInBlock;
    for(GLuint i = 0; i < numberOfUniformsInBlock; ++i)
    {
        std::string name(maxUniformNameLength, '\0');
        GLint size;
        GLenum type;

        glGetActiveUniform(shaderProgram
                           , (GLuint)activeUniformIndicies.get()[i]
                           , maxUniformNameLength
                           , nullptr
                           , &size
                           , &type
                           , &name[0]);

        name.erase(name.find_first_of('\0') + 1);

        GLchar* glName = new GLchar[name.size()];
        memcpy(glName, name.c_str(), name.size());
        uniformNames.push_back(glName);

        {
            auto findIndex = name.find_first_of('[');
            while(findIndex != name.npos)
            {
                name.erase(findIndex, name.find_first_of(']') - findIndex + 1);
                findIndex = name.find_first_of('[');
            }
        }

        auto iter = uniformsInBlock.find(name);
        if(iter == uniformsInBlock.end())
            uniformsInBlock.insert(Uniform(name, type, 0));
        else
            (*iter).count++;

        this->size += GLHelpers::Sizeof((GLEnums::UNIFORM_TYPE)type);
    }

//    for(const auto& uniform : uniformsInBlock)
//    {
//        Do this here
    //}

    data.reset(malloc(size));
    memset(data.get(), 0, size);

    // TODO: Support different indices and stuff

/*    // glGetActiveUniformsiv takes GLuints for indicies, while glGetActiveUniformBlockiv returns GLints...
    std::unique_ptr<GLuint> indices(new GLuint[numberOfUniformsInBlock]);
    glGetUniformIndices(shaderProgram, numberOfUniformsInBlock, &uniformNames[0], indices.get());

    std::unique_ptr<GLint> offsets(new GLint[numberOfUniformsInBlock]);
    glGetActiveUniformsiv(shaderProgram, numberOfUniformsInBlock, indices.get(), GL_UNIFORM_OFFSET, offsets.get());

    glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &size);*/

    glGenBuffers(1, &bufferIndex);

    glBindBuffer(GL_UNIFORM_BUFFER, bufferIndex);
    glBufferData(GL_UNIFORM_BUFFER, size, data.get(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformBlockBinding(shaderProgram, blockIndex, bindingPoint);

    return true;
}

void GLUniformBuffer::SetData(const void* data, size_t dataSize)
{
    memcpy(this->data.get(), data, dataSize);
    modifiedSinceCopy = true;
}

void GLUniformBufferVariable::CopyDataToParent(void* data)
{
    std::memcpy((char*)parent.data.get() + offset, data, GetSize());
    parent.modifiedSinceCopy = true;
}