#ifndef OBJMODEL_H__
#define OBJMODEL_H__

#include "content.h"
#include "../gl/glDrawBinds.h"

class Texture;

struct OBJModelParameters
        : ContentParameters
{
    std::string shaderPath;
    GLDrawBinds* forwardDrawBinds;
};

class OBJModel
        : public DiskContent
{
public:
    OBJModel();
    ~OBJModel();

    int GetStaticVRAMUsage() const override;
    int GetDynamicVRAMUsage() const override;
    int GetRAMUsage() const override;

    bool CreateDefaultContent(const char* filePath, ContentManager* contentManager) override;

    // FIXME
    //GLDrawBinds drawBinds; // TODO

    void DrawOpaque(int tileCountX, GLDrawBinds* drawBinds);
    void DrawTransparent(const glm::vec3 cameraPosition, GLDrawBinds* drawBinds);

    glm::mat4 worldMatrix;
protected:
    CONTENT_ERROR_CODES Load(const char* filePath
                             , ContentManager* contentManager
                             , ContentParameters* contentParameters) override;

    void Unload(ContentManager* contentManager) override;
    CONTENT_ERROR_CODES BeginHotReload(const char* filePath, ContentManager* contentManager) override;
    bool ApplyHotReload() override;
    bool Apply(Content* content) override;
    DiskContent* CreateInstance() const override;

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

    struct Material
    {
        glm::vec3 ambientColor; // Ka
        float specularExponent; // Ns

        glm::vec3 diffuseColor; // Kd
        float opacity;

        Texture* texture;
    };

    struct GPUMaterial
    {
        glm::vec3 ambientColor; // Ka
        float specularExponent; // Ns

        glm::vec3 diffuseColor; // Kd
        float opacity;

        GPUMaterial(const Material& other)
                : ambientColor(other.ambientColor)
                  , specularExponent(other.specularExponent)
                  , diffuseColor(other.diffuseColor)
                  , opacity(other.opacity)
        { }

        GPUMaterial()
        { }
    };
    struct DrawData
    {
        int materialIndex;
        int indexOffset;
        int indexCount;
        glm::vec3 centerPosition;
        float distanceToCamera;
    };

    std::vector<DrawData> opaqueDrawData;
    std::vector<DrawData> transparentDrawData;
    std::vector<Material> materials;

    GLIndexBuffer indexBuffer;
    GLVertexBuffer vertexBuffer;

};

#endif // OBJMODEL_H__
