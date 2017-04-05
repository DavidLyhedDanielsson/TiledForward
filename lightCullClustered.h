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
    void SetDrawBindData(GLDrawBinds& binds) override;

    std::string GetForwardShaderPath() override;
    std::string GetForwardShaderDebugPath() override;

private:
    typedef LightCullAdaptive Base;

    void Draw() override;

private:
    GLDrawBinds lightClusterDrawBinds;
};

#endif // LIGHTCULLCLUSTERED_H__
