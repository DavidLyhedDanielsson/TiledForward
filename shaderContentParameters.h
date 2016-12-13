#ifndef SHADERCONTENTPARAMETERS_H
#define SHADERCONTENTPARAMETERS_H

#include "content.h"

#include "glEnums.h"

struct ShaderContentParameters
        : public ContentParameters
{
    ShaderContentParameters()
            : type(GLEnums::SHADER_TYPE::UNKNOWN)
    {

    }

    ShaderContentParameters(GLEnums::SHADER_TYPE type)
            : type(type)
    {}

    GLEnums::SHADER_TYPE type;
};

#endif //SHADERCONTENTPARAMETERS_H
