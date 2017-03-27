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

    void InitShaderConstants(int screenWidth, int screenHeight) override;
    bool Init(ContentManager& contentManager, Console& console) override;
    void SetDrawBindData(GLDrawBinds& binds) override;

    void Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse) override;
    GLuint64 TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse) override;

    void DrawLightCount(SpriteRenderer& spriteRenderer
                            , CharacterSet* characterSetSmall
                            , CharacterSet* characterSetBig) override;

    void ResolutionChanged(int newWidth, int newHeight) override;

    glm::uvec2 GetThreadsPerGroup() const;

    int GetMaxLightsPerTile() const;
    int GetMaxNumberOfTreeIndices() const;
    int GetMaxNumberOfTiles() const;
    int GetTreeStartDepth() const;
    int GetTreeMaxDepth() const;

    GLDrawBinds lightCullDrawBinds;
    GLDrawBinds lightReductionDrawBinds;

protected:
private:
    const static int TREE_MAX_DEPTH = 8;

    int treeStartDepth = 1;
    int treeMaxDepth = 8;

    void PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse);
    void Draw();
    void PostDraw();
    int GetTreeDataScreen(int screenX, int screenY, int* tree);

    std::vector<glm::vec4> colors;

    GLCPPShared* sharedVariables;
};

#endif // LIGHTCULLADAPTIVE_H__
