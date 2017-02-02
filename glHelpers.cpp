#include "glHelpers.h"

#include <stdexcept>

namespace GLHelpers
{
    size_t Sizeof(GLEnums::DATA_TYPE type)
    {
        switch(type)
        {
            case GLEnums::DATA_TYPE::BYTE:
                return sizeof(GLbyte);
            case GLEnums::DATA_TYPE::UNSIGNED_BYTE:
                return sizeof(GLubyte);
            case GLEnums::DATA_TYPE::SHORT:
                return sizeof(GLshort);
            case GLEnums::DATA_TYPE::UNSIGNED_SHORT:
                return sizeof(GLushort);
            case GLEnums::DATA_TYPE::INT:
                return sizeof(GLint);
            case GLEnums::DATA_TYPE::UNSIGNED_INT:
                return sizeof(GLuint);
            case GLEnums::DATA_TYPE::FLOAT:
                return sizeof(GLfloat);
            case GLEnums::DATA_TYPE::BYTES_2:
            case GLEnums::DATA_TYPE::BYTES_3:
            case GLEnums::DATA_TYPE::BYTES_4:
                throw std::runtime_error("Not implemented");
            case GLEnums::DATA_TYPE::DOUBLE:
                return sizeof(GLdouble);
            case GLEnums::DATA_TYPE::UNKNOWN:
                return -1;
        }
    }

    size_t Sizeof(GLEnums::UNIFORM_TYPE type)
    {
        switch(type)
        {
            case GLEnums::UNIFORM_TYPE::FLOAT:
            case GLEnums::UNIFORM_TYPE::INT:
            case GLEnums::UNIFORM_TYPE::BOOL:
                return 4;
            case GLEnums::UNIFORM_TYPE::FLOAT_VEC2:
            case GLEnums::UNIFORM_TYPE::INT_VEC2:
            case GLEnums::UNIFORM_TYPE::BOOL_VEC2:
                return 8;
            case GLEnums::UNIFORM_TYPE::FLOAT_VEC3:
            case GLEnums::UNIFORM_TYPE::INT_VEC3:
            case GLEnums::UNIFORM_TYPE::BOOL_VEC3:
                return 12;
            case GLEnums::UNIFORM_TYPE::FLOAT_VEC4:
            case GLEnums::UNIFORM_TYPE::INT_VEC4:
            case GLEnums::UNIFORM_TYPE::BOOL_VEC4:
            case GLEnums::UNIFORM_TYPE::FLOAT_MAT2:
                return 16;
            case GLEnums::UNIFORM_TYPE::FLOAT_MAT3:
                return 36;
            case GLEnums::UNIFORM_TYPE::FLOAT_MAT4:
                return 64;
            case GLEnums::UNIFORM_TYPE::FLOAT_MAT2X3:
            case GLEnums::UNIFORM_TYPE::FLOAT_MAT3X2:
                return 24;
            case GLEnums::UNIFORM_TYPE::FLOAT_MAT2X4:
            case GLEnums::UNIFORM_TYPE::FLOAT_MAT4X2:
                return 32;
            case GLEnums::UNIFORM_TYPE::FLOAT_MAT3X4:
            case GLEnums::UNIFORM_TYPE::FLOAT_MAT4X3:
                return 48;
            case GLEnums::UNIFORM_TYPE::SAMPLER_2D:
            case GLEnums::UNIFORM_TYPE::SAMPLER_CUBE:
            case GLEnums::UNIFORM_TYPE::UNKNOWN:
                return -1;
        }
    }
};