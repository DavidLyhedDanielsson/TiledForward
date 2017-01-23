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
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 texCoord;
    };

    struct Material
    {
        glm::vec3 ambient;
        glm::vec3 diffuse;
        glm::vec3 specular;
        float specularExponent;

        std::string textureName;
    };

    struct OBJModel
    {
        std::map<std::string, Material> materials;
        std::map<std::string, std::vector<Vertex>> vertices;
    };

    auto ReadModel(const std::string& path) -> OBJModel;
}

#endif