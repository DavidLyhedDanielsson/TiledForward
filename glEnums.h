#ifndef GLENUMS_H
#define GLENUMS_H

#include <GL/glew.h>

namespace GLEnums
{
    enum class BUFFER_TYPE : GLuint
    {
        VERTEX = GL_ARRAY_BUFFER
        , INDEX = GL_ELEMENT_ARRAY_BUFFER
    };

    enum class BUFFER_USAGE : GLuint
    {
        STATIC = GL_STATIC_DRAW
        , DYNAMIC = GL_DYNAMIC_DRAW
        , STREAM = GL_STREAM_DRAW
    };

    enum class SHADER_TYPE : GLuint
    {
        VERTEX = GL_VERTEX_SHADER
        , TESS_CONTROL = GL_TESS_CONTROL_SHADER
        , TESS_EVALUATION = GL_TESS_EVALUATION_SHADER
        , GEOMETRY = GL_GEOMETRY_SHADER
        , PIXEL = GL_FRAGMENT_SHADER
        , COMPUTE = GL_COMPUTE_SHADER
    };

    enum class DATA_TYPE : GLuint
    {
        BYTE = GL_BYTE
        , UNSIGNED_BYTE = GL_UNSIGNED_BYTE
        , SHORT = GL_SHORT
        , UNSIGNED_SHORT = GL_UNSIGNED_SHORT
        , INT = GL_INT
        , UNSIGNED_INT = GL_UNSIGNED_INT
        , FLOAT = GL_FLOAT
        , BYTES_2 = GL_2_BYTES
        , BYTES_3 = GL_3_BYTES
        , BYTES_4 = GL_4_BYTES
        , DOUBLE = GL_DOUBLE
    };

    enum class UNIFORM_TYPE : GLuint
    {
        FLOAT = GL_FLOAT
        , FLOAT_VEC2 = GL_FLOAT_VEC2
        , FLOAT_VEC3 = GL_FLOAT_VEC3
        , FLOAT_VEC4 = GL_FLOAT_VEC4
        , INT = GL_INT
        , INT_VEC2 = GL_INT_VEC2
        , INT_VEC3 = GL_INT_VEC3
        , INT_VEC4 = GL_INT_VEC4
        , BOOL = GL_BOOL
        , BOOL_VEC2 = GL_BOOL_VEC2
        , BOOL_VEC3 = GL_BOOL_VEC3
        , BOOL_VEC4 = GL_BOOL_VEC4
        , FLOAT_MAT2 = GL_FLOAT_MAT2
        , FLOAT_MAT3 = GL_FLOAT_MAT3
        , FLOAT_MAT4 = GL_FLOAT_MAT4
        , SAMPLER_2D = GL_SAMPLER_2D
        , SAMPLER_CUBE = GL_SAMPLER_CUBE
        , FLOAT_MAT2X3 = GL_FLOAT_MAT2x3
        , FLOAT_MAT3X2 = GL_FLOAT_MAT3x2
        , FLOAT_MAT2X4 = GL_FLOAT_MAT2x4
        , FLOAT_MAT4X2 = GL_FLOAT_MAT4x2
        , FLOAT_MAT3X4 = GL_FLOAT_MAT3x4
        , FLOAT_MAT4X3 = GL_FLOAT_MAT4x3
    };
}

#endif //GLENUMS_H
