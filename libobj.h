#ifndef OBJ_LIBRARY_H
#define OBJ_LIBRARY_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <vector>
#include <string>
#include <map>

namespace LibOBJ
{
    struct Vertex
    {
        Vertex()
        {}

        Vertex(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord)
                : position(position), normal(normal), texCoord(texCoord)
        {}

        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };
}

#endif