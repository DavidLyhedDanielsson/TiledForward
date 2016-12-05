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
};