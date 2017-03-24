#ifndef LIGHTCULL_H__
#define LIGHTCULL_H__

#include "glDrawBinds.h"

#include "characterSet.h"
#include "spriteRenderer.h"

class LightCull
{
public:
    LightCull();
    ~LightCull();

    void InitShaderConstants(int screenWidth, int screenHeight);
    bool Init(ContentManager& contentManager);

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

    GLShaderStorageBuffer* GetActiveTileLightsData();

protected:
private:
    const static int MAX_LIGHTS_PER_TILE = 1;
    const static int TREE_START_DEPTH = 1;
    const static int TREE_MAX_DEPTH = 6;

    static_assert(TREE_START_DEPTH >= 1, "TREE_START_DEPTH < 1");
    static_assert(TREE_MAX_DEPTH >= TREE_START_DEPTH, "TREE_MAX_DEPTH < TREE_START_DEPTH");

    //const glm::uvec2 workGroupSize; // In pixels
    const glm::uvec2 threadsPerGroup;
    //glm::uvec2 workGroupCount;

    int screenWidth;
    int screenHeight;

    GLuint timeQuery;

    std::unique_ptr<GLShaderStorageBuffer> tileLightData[2];

    void PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse);
    void Draw();
    void PostDraw();
    int GetTreeDataScreen(int screenX, int screenY, int* tree);
    int activeTileLightsData;
};

#endif // LIGHTCULL_H__
