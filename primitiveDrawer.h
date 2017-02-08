#ifndef PRIMITIVEDRAWER_H__
#define PRIMITIVEDRAWER_H__

#include "glDrawBinds.h"
#include "libobj.h"

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
