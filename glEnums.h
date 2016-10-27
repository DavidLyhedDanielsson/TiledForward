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
}

#endif //GLENUMS_H
