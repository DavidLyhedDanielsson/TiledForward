#ifndef SHADERCONTENTPARAMETERS_H
#define SHADERCONTENTPARAMETERS_H

#include "content.h"

#include "../gl/glEnums.h"

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

    std::vector<std::pair<std::string, std::string>> variables;
};

#endif //SHADERCONTENTPARAMETERS_H
