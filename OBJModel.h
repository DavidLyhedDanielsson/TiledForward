#ifndef OBJMODEL_H__
#define OBJMODEL_H__

#include "content.h"
#include "glDrawBinds.h"
#include "libobj.h"

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

    int drawOnlyIndex = -1;

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
    std::vector<std::pair<LibOBJ::Material, std::pair<int, int>>> materialOffset;

    GLIndexBuffer indexBuffer;
    GLVertexBuffer vertexBuffer;
};

#endif // OBJMODEL_H__
