#ifndef LIGHTCULLADAPTIVE_H__
#define LIGHTCULLADAPTIVE_H__

#include "lightCull.h"
#include "gl/glDrawBinds.h"

#include "content/characterSet.h"
#include "spriteRenderer.h"
#include "console/console.h"
#include "gl/glCPPShared.h"

class LightCullAdaptive
    : public LightCull
{
public:
    LightCullAdaptive();
    ~LightCullAdaptive();

    virtual void InitShaderConstants(int screenWidth, int screenHeight) override;
    virtual bool Init(ContentManager& contentManager, Console& console) override;
    virtual void SetDrawBindData(GLDrawBinds& binds) override;

    virtual void Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager) override;
    virtual GLuint64 TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager) override;

    virtual void DrawLightCount(SpriteRenderer& spriteRenderer
                            , CharacterSet* characterSetSmall
                            , CharacterSet* characterSetBig) override;

    virtual void ResolutionChanged(int newWidth, int newHeight) override;

    int GetMaxLightsPerTile() const;
    int GetMaxNumberOfTreeIndices() const;
    int GetMaxNumberOfTiles() const;
    int GetTreeStartDepth() const;
    int GetTreeMaxDepth() const;

    GLDrawBinds lightCullDrawBinds;
    GLDrawBinds lightReductionDrawBinds;

    virtual std::string GetForwardShaderPath() override;
    virtual std::string GetForwardShaderDebugPath() override;
protected:
    const static int TREE_MAX_DEPTH = 6;

    int treeStartDepth = 1;
    int treeMaxDepth = TREE_MAX_DEPTH;

    virtual void PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager);
    virtual void Draw();
    virtual void PostDraw();
    int GetTreeDataScreen(int screenX, int screenY, int* tree);

    std::vector<glm::vec4> colors;

    GLCPPShared* sharedVariables;
};

#endif // LIGHTCULLADAPTIVE_H__
