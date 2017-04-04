#ifndef LIGHTCULL_H__
#define LIGHTCULL_H__

#include <GL/gl3w.h>

#include "content/contentManager.h"
#include "console/console.h"
#include "gl/glDrawBinds.h"

class LightCull
{
public:
    LightCull();
    virtual ~LightCull();

    virtual void InitShaderConstants(int screenWidth, int screenHeight) = 0;
    virtual bool Init(ContentManager& contentManager, Console& console) = 0;

    virtual void SetDrawBindData(GLDrawBinds& binds) = 0;

    virtual void Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse) = 0;
    virtual GLuint64 TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse) = 0;

    virtual void DrawLightCount(SpriteRenderer& spriteRenderer
                                , CharacterSet* characterSetSmall
                                , CharacterSet* characterSetBig) = 0;

    virtual void ResolutionChanged(int newWidth, int newHeight) = 0;

    virtual std::string GetForwardShaderPath() = 0;
    virtual std::string GetForwardShaderDebugPath() = 0;
protected:
    int screenWidth;
    int screenHeight;

    GLuint timeQuery;

    const static int MAX_LIGHTS_PER_TILE = 1024;
};

#endif // LIGHTCULL_H__
