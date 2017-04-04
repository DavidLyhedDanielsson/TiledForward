#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "primitiveDrawer.h"

PrimitiveDrawer::PrimitiveDrawer()
{}

PrimitiveDrawer::~PrimitiveDrawer()
{}

bool PrimitiveDrawer::Init(ContentManager& contentManager)
{
    GenerateSphere(contentManager, 1.0f, 51, 51);

    return true;
}

void PrimitiveDrawer::GenerateSphere(ContentManager& contentManager
                                     , float radius
                                     , int widthSegments
                                     , int heightSegments)
{
    std::vector<Vertex> vertices;
    std::vector<GLint> indices;

    //////////////////////////////////////////////////
    //Generate vertices
    //////////////////////////////////////////////////
    float angle = glm::two_pi<float>() / static_cast<float>(widthSegments);
    float heightSegmentInc = radius * 2.0f / static_cast<float>(heightSegments + 1);

    //Top
    vertices.push_back(Vertex(glm::vec3(0.0f, radius, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)));

    float yPos = radius - heightSegmentInc;

    float power = 3.0;

    //float preDiv = 1 / (1 - 1 / (std::pow(glm::e<float>(), power)));
    float preDiv = 1 / (1 - 1 / (std::exp(power)));

    for(int y = 0; y < std::ceil(heightSegments * 0.5f); ++y)
    {
        float percent = yPos / radius;

        //float first = 1 - std::pow(glm::e<float>(), -power * percent);
        float first = 1 - std::exp(-power * percent);
        float actualYPos = first * preDiv * radius;

        for(int x = 0; x < widthSegments; ++x)
        {
            glm::vec2 normalizedXZ = glm::normalize(glm::vec2(std::cos(x * angle), std::sin(x * angle)));

            float wantedXZDist = std::sqrt((radius * radius) - (actualYPos * actualYPos));

            glm::vec3 newPos(normalizedXZ.x * wantedXZDist, actualYPos, normalizedXZ.y * wantedXZDist);

            //newPos = normalizedXZ * wantedXZDist;
            //newPos.y = yPos;

            //a² + b² = c²
            //c = radius, a = yPos =>
            //yPos² + b² = radius²
            //b² = radius² - yPos²
            //b = sqrt(radius² - yPos²)

            //       xz (b)
            //        _____
            //       \    |
            //	      \   |
            //xyz (c)  \  | y (a)
            //          \ |
            //           \|

            vertices.push_back(Vertex(newPos, glm::vec3(0, 0, 0), glm::vec2(0, 0)));
        }

        yPos -= heightSegmentInc;
    }

    //Code duplication is bad, mmkay?
    for(int y = static_cast<int>(std::ceil(heightSegments * 0.5f) + 1); y <= heightSegments; ++y)
    {
        float percent = -yPos / radius;

        //float first = 1 - std::pow(glm::e<float>(), -power * percent);
        float first = 1 - std::exp(-power * percent);
        float actualYPos = -first * preDiv * radius;

        for(int x = 0; x < widthSegments; ++x)
        {
            glm::vec2 normalizedXZ = glm::normalize(glm::vec2(std::cos(x * angle), std::sin(x * angle)));

            float wantedXZDist = std::sqrt((radius * radius) - (actualYPos * actualYPos));

            glm::vec3 newPos(normalizedXZ.x * wantedXZDist, actualYPos, normalizedXZ.y * wantedXZDist);

            vertices.push_back(Vertex(newPos, glm::vec3(0.0f), glm::vec2(0.0f)));
        }

        yPos -= heightSegmentInc;
    }

    //Bottom
    vertices.push_back(Vertex(glm::vec3(0.0f, -radius, 0.0f), glm::vec3(0.0f), glm::vec2(0.0f)));

    //////////////////////////////////////////////////
    //Generate indicies
    //////////////////////////////////////////////////
    //Top
    for(int i = 1; i < widthSegments; ++i)
    {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    //Connect first face with last face
    indices.push_back(0);
    indices.push_back(widthSegments);
    indices.push_back(1);


    //Body
    for(int y = 0; y < heightSegments - 1; ++y)
    {
        for(int x = 1; x <= widthSegments; ++x)
        {
            int beginIndex = widthSegments * y;

            indices.push_back(beginIndex + x);
            indices.push_back((widthSegments * 1) + (beginIndex + 1) + x % widthSegments);
            indices.push_back((beginIndex + 1) + x % widthSegments);

            indices.push_back(beginIndex + x);
            indices.push_back(beginIndex + x + widthSegments);
            indices.push_back((widthSegments * 1) + (beginIndex + 1) + x % widthSegments);
        }
    }

    int beginIndex = widthSegments * (heightSegments - 1);

    //Bottom
    for(int i = 1; i < widthSegments; ++i)
    {
        indices.push_back(static_cast<int>(vertices.size()) - 1);
        indices.push_back(beginIndex + i + 1);
        indices.push_back(beginIndex + i);
    }

    //Connect first face with last face
    indices.push_back(static_cast<int>(vertices.size()) - 1);
    indices.push_back(beginIndex + 1);
    indices.push_back(static_cast<int>(vertices.size()) - 2);

    //////////////////////////////////////////////////
    //Normals
    //////////////////////////////////////////////////
    for(Vertex& vertex : vertices)
        vertex.normal = glm::normalize(vertex.position);

    sphereVertexBuffer.Init<Vertex, glm::vec3, glm::vec3, glm::vec2>(GLEnums::BUFFER_USAGE::STATIC_DRAW, vertices);
    sphereIndexBuffer.Init(GLEnums::BUFFER_USAGE::STATIC_DRAW, indices);

    GLInputLayout vertexInputLayout;
    vertexInputLayout.SetInputLayout<glm::vec3, glm::vec3, glm::vec2>();

    std::vector<InstanceData> instanceData;

    sphereInstanceBuffer.Init<InstanceData, glm::mat4, glm::vec4>(GLEnums::BUFFER_USAGE::STREAM_DRAW, nullptr, MAX_SPHERES);

    GLInputLayout instanceInputLayout;
    instanceInputLayout.SetInputLayout<glm::vec4, glm::mat4>(3);
    instanceInputLayout.SetVertexAttribDivisor(3, 1); // color
    instanceInputLayout.SetVertexAttribDivisor(4, 1); // world matrix
    instanceInputLayout.SetVertexAttribDivisor(5, 1); // world matrix
    instanceInputLayout.SetVertexAttribDivisor(6, 1); // world matrix
    instanceInputLayout.SetVertexAttribDivisor(7, 1); // worldMatrix

    sphereBinds.AddBuffers(&sphereVertexBuffer, vertexInputLayout
                         , &sphereInstanceBuffer, instanceInputLayout
                         , &sphereIndexBuffer);

    sphereBinds.AddShaders(contentManager
                         , GLEnums::SHADER_TYPE::VERTEX, "rendering/instance.vert"
                         , GLEnums::SHADER_TYPE::FRAGMENT, "rendering/instance.frag");

    sphereBinds.AddUniform("viewProjectionMatrix", glm::mat4());

    sphereBinds.Init();
}

void PrimitiveDrawer::Begin()
{

}

void PrimitiveDrawer::End()
{
    if(spheres.empty())
        return;

    sphereInstanceBuffer.Update(&spheres[0], sizeof(InstanceData) * spheres.size());

    sphereBinds.Bind();

    sphereBinds.DrawElementsInstanced(spheres.size());

    sphereBinds.Unbind();

    spheres.clear();
}

void PrimitiveDrawer::DrawSphere(glm::vec3 position, float size, glm::vec3 color)
{
    InstanceData instanceData;

    instanceData.color = glm::vec4(color, 1.0f);
    instanceData.worldMatrix = glm::translate(glm::mat4(), position);
    instanceData.worldMatrix = glm::scale(instanceData.worldMatrix, glm::vec3(size));

    spheres.push_back(instanceData);
}
