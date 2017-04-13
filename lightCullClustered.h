#ifndef LIGHTCULLCLUSTERED_H__
#define LIGHTCULLCLUSTERED_H__

#include "gl/glDrawBinds.h"

#include "content/characterSet.h"
#include "spriteRenderer.h"
#include "console/console.h"
#include "gl/glCPPShared.h"
#include "lightCullAdaptive.h"

class LightCullClustered
    : public LightCullAdaptive
{
public:
    LightCullClustered();
    ~LightCullClustered();

    bool Init(ContentManager& contentManager, Console& console) override;
    void SetDrawBindData() override;

    //std::string GetForwardShaderPath() override;
    //std::string GetForwardShaderDebugPath() override;

    int GetTileCountX() const;

private:
    typedef LightCullAdaptive Base;

    void Draw() override;
    void PostDraw() override;

private:
    GLDrawBinds lightSortDrawBinds;

    bool sort;
};

#endif // LIGHTCULLCLUSTERED_H__
