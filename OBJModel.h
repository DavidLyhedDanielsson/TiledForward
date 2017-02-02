#ifndef OBJMODEL_H__
#define OBJMODEL_H__

#include "content.h"
#include "glDrawBinds.h"
#include "libobj.h"

class Texture;

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
    GLDrawBinds drawBinds; // TODO

    void Draw();

    int materialIndex = 0;

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
    };

    std::vector<DrawData> drawData;
    std::vector<Material> materials;

    GLIndexBuffer indexBuffer;
    GLVertexBuffer vertexBuffer;

    glm::mat4 worldMatrix;
};

#endif // OBJMODEL_H__
