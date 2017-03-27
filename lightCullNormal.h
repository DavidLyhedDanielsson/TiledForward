#ifndef LIGHTCULLNORMAL_H__
#define LIGHTCULLNORMAL_H__

#include "lightCull.h"
#include "gl/glCPPShared.h"

class LightCullNormal
        : public LightCull
{
public:
    LightCullNormal();
    ~LightCullNormal();

    void InitShaderConstants(int screenWidth, int screenHeight);
    bool Init(ContentManager& contentManager, Console& console);
    void SetDrawBindData(GLDrawBinds& binds);

    void Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse);
    GLuint64 TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse);

    void DrawLightCount(SpriteRenderer& spriteRenderer
                        , CharacterSet* characterSetSmall
                        , CharacterSet* characterSetBig) override;

    void ResolutionChanged(int newWidth, int newHeight);

    glm::uvec2 GetThreadsPerGroup() const;

    int GetMaxLightsPerTile() const;
    int GetMaxNumberOfTiles() const;

    GLDrawBinds lightCullDrawBinds;

protected:
private:
    const static int MAX_DEPTH = 8;

    int maxDepth = 2;
    glm::ivec2 threadGroupCount;

    void PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse);
    void Draw();
    void PostDraw();

    GLCPPShared* sharedVariables;
};

#endif // LIGHTCULL_H__
