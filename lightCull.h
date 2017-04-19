#ifndef LIGHTCULL_H__
#define LIGHTCULL_H__

#include <GL/gl3w.h>

#include "content/contentManager.h"
#include "console/console.h"
#include "gl/glDrawBinds.h"
#include "lightManager.h"
#include "perspectiveCamera.h"
#include "content/OBJModel.h"

class LightCull
{
public:
    LightCull();
    virtual ~LightCull();

    virtual void InitShaderConstants(int screenWidth, int screenHeight) = 0;
    virtual bool Init(ContentManager& contentManager, Console& console) = 0;

    virtual void SetDrawBindData() = 0;

    virtual void Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager) = 0;
    virtual std::vector<std::pair<std::string, GLuint64>> TimedDraw(glm::mat4 viewMatrix
                                                                    , glm::mat4 projectionMatrixInverse
                                                                    , LightManager& lightManager) = 0;

    virtual void DrawLightCount(SpriteRenderer& spriteRenderer
                                , CharacterSet* characterSetSmall
                                , CharacterSet* characterSetBig) = 0;

    virtual void ResolutionChanged(int newWidth, int newHeight) = 0;

    virtual GLDrawBinds* GetForwardDrawBinds() = 0;

    //virtual std::string GetForwardShaderPath() = 0;
    //virtual std::string GetForwardShaderDebugPath() = 0;
    virtual int GetTileCountX() const;
    virtual void UpdateUniforms(PerspectiveCamera* pCamera, OBJModel* pModel, LightManager* lightManager) = 0;
protected:
    int screenWidth;
    int screenHeight;

    // Must be power of two
    const static int MAX_LIGHTS_PER_TILE = 4;

    void StartTimeQuery();
    GLuint64 StopTimeQuery();

private:
    GLuint timeQuery;
};

#endif // LIGHTCULL_H__
