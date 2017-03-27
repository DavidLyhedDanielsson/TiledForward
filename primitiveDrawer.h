#ifndef PRIMITIVEDRAWER_H__
#define PRIMITIVEDRAWER_H__

#include "gl/glDrawBinds.h"

class PrimitiveDrawer
{
public:
    PrimitiveDrawer();
    ~PrimitiveDrawer();

    bool Init(ContentManager& contentManager);

    GLDrawBinds sphereBinds; // FIXME

    void Begin();
    void End();

    void DrawSphere(glm::vec3 position, float size, glm::vec3 color);

protected:
private:
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

    const static int MAX_SPHERES = 2048;

    struct InstanceData
    {
        glm::vec4 color;
        glm::mat4 worldMatrix;
    };

    GLVertexBuffer sphereVertexBuffer;
    GLVertexBuffer sphereInstanceBuffer;
    GLIndexBuffer sphereIndexBuffer;

    // <position, radius>
    std::vector<InstanceData> spheres;

    void GenerateSphere(ContentManager& contentManager, float radius, int widthSegments, int heightSegments);
};

#endif // PRIMITIVEDRAWER_H__
