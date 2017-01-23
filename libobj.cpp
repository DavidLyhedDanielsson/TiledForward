#include "libobj.h"

#include <iostream>
#include <fstream>
#include <sstream>

using namespace LibOBJ;

const static char* const numericVariables = "0123456789.-";

auto TrimString(std::string& string) -> void;
auto BeginsWith(const std::string& string, const std::string& beginsWith) -> bool;

void GetTriangles(const std::string& line
                  , std::vector<Vertex>& vertices
                  , const std::vector<glm::vec3>& positions
                  , const std::vector<glm::vec3>& normals
                  , const std::vector<glm::vec2>& texCoords);

auto GetMtllib(const std::string& mtllibPath) -> std::map<std::string, Material>;
auto GetVec3(const std::string& line) -> glm::vec3;
auto GetVec2(const std::string& line) -> glm::vec2;
auto GetFloat(const std::string& line) -> float;
auto GetString(const std::string& line) -> std::string;

auto LibOBJ::ReadModel(const std::string& path) -> OBJModel
{
    std::ifstream in(path);
    if(!in.is_open())
        throw std::runtime_error("Couldn't open file");

    OBJModel returnModel;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;

    normals.push_back({0.0f, 0.0f, 0.0f});
    texCoords.push_back({0.0f, 0.0f});

    std::string currentMaterial = "";

    std::string line;
    while(std::getline(in, line))
    {
        TrimString(line);

        if(line[0] == '#')
            continue;

        if(BeginsWith(line, "v "))
            positions.push_back(GetVec3(line));
        else if(BeginsWith(line, "vn "))
            normals.push_back(GetVec3(line));
        else if(BeginsWith(line, "vt "))
            texCoords.push_back(GetVec2(line));
        else if(BeginsWith(line, "f "))
            GetTriangles(line, returnModel.vertices[currentMaterial], positions, normals, texCoords);
        else if(BeginsWith(line, "usemtl "))
            currentMaterial = line.substr(line.find_first_of(' ') + 1);
        else if(BeginsWith(line, "mtllib "))
        {
            std::string pathPrefix = path.substr(0, path.find_last_of('/') + 1);
            std::string mtillibName = line.substr(line.find_first_of(' ') + 1);

            returnModel.materials = GetMtllib(pathPrefix + mtillibName);
        }
    }

    return returnModel;
}

auto TrimString(std::string& string) -> void
{
    string.erase(0, string.find_first_not_of(" \t"));
    string.erase(string.find_last_not_of(" \t") + 1);
}

auto BeginsWith(const std::string& string, const std::string& beginsWith) -> bool
{
    return string.compare(0, beginsWith.size(), beginsWith) == 0;
}

void GetTriangles(const std::string& line
                  , std::vector<Vertex>& vertices
                  , const std::vector<glm::vec3>& positions
                  , const std::vector<glm::vec3>& normals
                  , const std::vector<glm::vec2>& texCoords)
{
    std::vector<Vertex> returnVector;

    size_t spaceIndex = line.find_first_of(' '); // Skip "f "
    size_t lastSpaceIndex = 0;
    do
    {
        lastSpaceIndex = spaceIndex;
        spaceIndex = line.find_first_of(' ', spaceIndex + 1);

        std::stringstream stringstream(line.substr(lastSpaceIndex + 1, spaceIndex - lastSpaceIndex - 1));

        std::vector<int> properties;
        std::string property;
        while(std::getline(stringstream, property, '/'))
            properties.push_back(property.empty() ? 0 : std::stoi(property));

        // Pad with 0
        properties.insert(properties.end(), 3 - properties.size(), 0);

        Vertex newVertex;
        // Order of these set by .obj file spec
        newVertex.position = positions[properties[0] - 1]; // No default position available, so 0-index it
        newVertex.texCoord = texCoords[properties[1]];
        newVertex.normal = normals[properties[2]];

        returnVector.push_back(newVertex);
    } while(spaceIndex != line.npos);

    // Create two triangles from quad
    if(returnVector.size() == 4)
    {
        // Winding order is CCW
        returnVector.insert(returnVector.begin() + 3, returnVector[0]);
        returnVector.push_back(returnVector[2]);
    }

    // Change winding order to CCW
    std::swap(returnVector[1], returnVector[2]);

    vertices.insert(vertices.end(), returnVector.begin(), returnVector.end());
}

auto GetMtllib(const std::string& mtllibPath) -> std::map<std::string, Material>
{
    std::ifstream in(mtllibPath);
    if(!in.is_open())
        throw std::runtime_error("Couldn't open file \"" + mtllibPath + "\"");

    std::map<std::string, Material> returnMap;

    // Data is added to this and then this is inserted into returnMap when "newmtl" is found
    std::string materialName;
    Material newMaterial;

    std::string line;
    while(std::getline(in, line))
    {
        TrimString(line);

        if(BeginsWith(line, "#"))
            continue;

        if(BeginsWith(line, "newmtl "))
        {
            if(!materialName.empty())
            {
                returnMap[materialName] = newMaterial;

                materialName = line.substr(line.find_first_not_of(' ') + 1);
                newMaterial = Material();
            }
        }
        else if(BeginsWith(line, "Ka"))
            newMaterial.ambient = GetVec3(line);
        else if(BeginsWith(line, "Kd"))
            newMaterial.diffuse = GetVec3(line);
        else if(BeginsWith(line, "Ks"))
            newMaterial.specular = GetVec3(line);
        else if(BeginsWith(line, "Ns"))
            newMaterial.specularExponent = GetFloat(line);
        else if(BeginsWith(line, "map_Ka"))
            newMaterial.textureName = GetString(line);
    }

    if(!materialName.empty())
        returnMap[materialName] = newMaterial;

    return returnMap;
}

auto GetVec3(const std::string& line) -> glm::vec3
{
    auto firstDigit = line.find_first_of(numericVariables);
    auto firstSpace = line.find_first_not_of(numericVariables, firstDigit + 1);

    auto secondDigit = line.find_first_of(numericVariables, firstSpace + 1);
    auto secondSpace = line.find_first_not_of(numericVariables, secondDigit + 1);

    auto thirdDigit = line.find_first_of(numericVariables, secondSpace + 1);

    return {
            std::atof(&line[firstDigit])
            , std::atof(&line[secondDigit])
            , std::atof(&line[thirdDigit])
    };
}

auto GetVec2(const std::string& line) -> glm::vec2
{
    auto firstDigit = line.find_first_of(numericVariables);
    auto firstSpace = line.find_first_not_of(numericVariables, firstDigit + 1);

    auto secondDigit = line.find_first_of(numericVariables, firstSpace + 1);

    return {
            std::atof(&line[firstDigit])
            , std::atof(&line[secondDigit])
    };
}

auto GetFloat(const std::string& line) -> float
{
    auto firstDigit = line.find_first_of(numericVariables);

    return (float)std::atof(&line[firstDigit]);
}

auto GetString(const std::string& line) -> std::string
{
    auto firstDigit = line.find_first_not_of(' ');

    return line.substr(firstDigit);
}
