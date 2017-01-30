#include "glUniformBlock.h"

#include <algorithm>

void GLUniformBlock::Init(const std::string& name, int size)
{
    this->name = name;
    this->size = size;
    this->data.reset(malloc(size));
}

GLUniformBlock::~GLUniformBlock()
{ }

void GLUniformBlock::UploadDataIfNeeded()
{
    if(modifiedSinceCopy)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, bufferIndex);
        glBufferSubData(GL_UNIFORM_BUFFER, blockIndex, size, data.get());
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

bool GLUniformBlock::Init(GLuint shaderProgram)
{
    blockIndex = glGetUniformBlockIndex(shaderProgram, name.c_str());

    if(blockIndex == (GLuint)-1)
        return false;

    // Get number of active uniforms and their indicies
    GLint numberOfUniformsInBlock;
    glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS, &numberOfUniformsInBlock);

    std::unique_ptr<GLint> activeUniformIndicies(new GLint[numberOfUniformsInBlock]);
    glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, activeUniformIndicies.get());

    // Get name of all active uniforms
    GLint maxUniformNameLength;
    glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformNameLength);

    // Add all active uniforms that are inside of this block to uniformsInBlock
    std::vector<std::string> uniformsInBlock;
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
        name.resize(name.find_first_of('\0'));

        uniformsInBlock.push_back(name);
    }

    if(numberOfUniformsInBlock > uniforms.size())
    {
        std::string errorMessage = "Number of uniforms in uniforms buffer objects is bigger than number of defined uniforms ("
                                   + std::to_string(numberOfUniformsInBlock)
                                   + " > "
                                   + std::to_string(uniforms.size())
                                   + ")\nThe following uniforms aren't defined:\n";

        for(const auto& uniform : uniformsInBlock)
            if(uniforms.count(uniform) == 0)
                errorMessage += "\t" + uniform + "\n";

        errorMessage.pop_back(); // Remove newline

        Logger::LogLine(LOG_TYPE::DEBUG, errorMessage);
    }
    else if(numberOfUniformsInBlock < uniforms.size())
    {
        std::string errorMessage = "Number of uniforms in uniforms buffer objects is smaller than number of defined uniforms ("
                + std::to_string(numberOfUniformsInBlock)
                + " < "
                + std::to_string(uniforms.size())
                + ")\nThe following uniforms will be ignored:\n";

        for(const auto& pair : uniforms)
            if(std::find(uniformsInBlock.begin(), uniformsInBlock.end(), pair.first) == uniformsInBlock.end())
                errorMessage += "\t" + pair.first + "\n";

        errorMessage.pop_back(); // Remove newline

        Logger::LogLine(LOG_TYPE::WARNING, errorMessage);
    }
    else
    {
        std::string nonMatchingUniforms;

        for(const auto& uniform : uniformsInBlock)
            if(uniforms.count(uniform) == 0)
                nonMatchingUniforms += "\t" + uniform + "\n";

        if(!nonMatchingUniforms.empty())
        {
            nonMatchingUniforms.pop_back(); // Remove newline

            Logger::LogLine(LOG_TYPE::WARNING, "Not all uniforms in uniform buffer object matches a defined uniform. "
                            "The following uniforms aren't defined:\n"
                            , nonMatchingUniforms);
        }
    }

    std::vector<const GLchar*> uniformNames;
    uniformNames.reserve(numberOfUniformsInBlock);

    for(const auto& uniform : uniformsInBlock)
        uniformNames.push_back(uniform.c_str());

    // TODO: Support different indices and stuff

    // glGetActiveUniformsiv takes GLuints for indicies, while glGetActiveUniformBlockiv returns GLints...
    std::unique_ptr<GLuint> indices(new GLuint[numberOfUniformsInBlock]);
    glGetUniformIndices(shaderProgram, numberOfUniformsInBlock, &uniformNames[0], indices.get());

    std::unique_ptr<GLint> offsets(new GLint[numberOfUniformsInBlock]);
    glGetActiveUniformsiv(shaderProgram, numberOfUniformsInBlock, indices.get(), GL_UNIFORM_OFFSET, offsets.get());

    auto oldSize = size;
    std::unique_ptr<void, uniquePtrFree> oldData(data.release());

    glGetActiveUniformBlockiv(shaderProgram, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &size); // TODO: Make sure size is correct!

    data.reset(malloc(size));

    int i = 0;
    for(const auto& name : uniformNames)
    {
        std::string version(name);
        const auto actualName = version.substr(0, version.find_first_of("["));

        auto oldOffset = uniforms.at(actualName).offset;
        auto newOffset = offsets.get()[i];

        memcpy((char*)data.get() + newOffset, (char*)oldData.get() + oldOffset, uniforms.at(actualName).GetSize());

        uniforms.at(actualName).offset = newOffset;

        ++i;
    }

    glGenBuffers(1, &bufferIndex);

    glBindBuffer(GL_UNIFORM_BUFFER, bufferIndex);
    glBufferData(GL_UNIFORM_BUFFER, size, data.get(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    return true;
}

void GLUniformBlockVariable::CopyDataToParent(void* data)
{
    std::memcpy((char*)parent.data.get() + offset, data, GetSize());
    parent.modifiedSinceCopy = true;
}