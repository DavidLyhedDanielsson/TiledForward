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

    std::vector<std::pair<std::string, GLuint64>> TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager) override;

    //std::string GetForwardShaderPath() override;
    //std::string GetForwardShaderDebugPath() override;

    int GetTileCountX() const;

private:
    typedef LightCullAdaptive Base;

    void Draw(std::vector<std::pair<std::string, GLuint64>>& times) override;
    void PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager) override;
    void PostDraw() override;

private:
    GLDrawBinds lightSortDrawBinds;
    GLDrawBinds lightClusterDrawBinds;

    const int BUCKET_COUNT = 4;

    bool sort;
};

#endif // LIGHTCULLCLUSTERED_H__
