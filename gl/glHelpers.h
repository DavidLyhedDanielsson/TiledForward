#ifndef GLHELPERS_H__
#define GLHELPERS_H__

#include <cstdlib>

#include "glEnums.h"

namespace GLHelpers
{
    size_t Sizeof(GLEnums::DATA_TYPE type);
    size_t Sizeof(GLEnums::UNIFORM_TYPE type);
}

#endif //GLHELPERS_H__
