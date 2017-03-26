#ifndef LIGHTCULL_H__
#define LIGHTCULL_H__

#include "glDrawBinds.h"

#include "characterSet.h"
#include "spriteRenderer.h"
#include "console/console.h"

class LightCull
{
public:
    LightCull();
    ~LightCull();

    void InitShaderConstants(int screenWidth, int screenHeight);
    bool Init(ContentManager& contentManager, Console& console);

    void Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse);
    GLuint64 TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse);

    void DrawLightCount(SpriteRenderer& spriteRenderer
                            , CharacterSet* characterSetSmall
                            , CharacterSet* characterSetBig);

    void ResolutionChanged(int newWidth, int newHeight);

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
    const static int MAX_LIGHTS_PER_TILE = 512;
    const static int TREE_MAX_DEPTH = 8;

    const glm::uvec2 threadsPerGroup;

    int treeStartDepth = 1;
    int treeMaxDepth = 8;

    int screenWidth;
    int screenHeight;

    GLuint timeQuery;

    void PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse);
    void Draw();
    void PostDraw();
    int GetTreeDataScreen(int screenX, int screenY, int* tree);
};

#endif // LIGHTCULL_H__
