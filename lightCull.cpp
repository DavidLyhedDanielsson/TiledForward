#include "lightCull.h"

#include "GL/gl3w.h"

LightCull::LightCull()
        : workGroupSize(128, 128)
          , threadsPerGroup(16, 16)
          , maxLightsPerTile(32)
{}

LightCull::~LightCull()
{}

void LightCull::InitShaderConstants(int screenWidth, int screenHeight)
{
    workGroupCount.x = (GLuint)std::ceil(screenWidth / (float)workGroupSize.x);
    workGroupCount.y = (GLuint)std::ceil(screenHeight / (float)workGroupSize.y);

    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
}

bool LightCull::Init(ContentManager& contentManager)
{
    ////////////////////////////////////////////////////////////
    // Make sure there aren't too many lights per tile

    GLint maxSize;
    glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxSize);

    if(maxLightsPerTile + 1 > maxSize / sizeof(int))
    {
        Logger::LogLine(LOG_TYPE::FATAL, "GPU only supports a maximum of ", maxSize / sizeof(int), " ints in shared memory");
        return false;
    }

    drawBinds.AddUniform("viewMatrix", glm::mat4());
    drawBinds.AddUniform("projectionInverseMatrix", glm::mat4());
    drawBinds.AddShaders(contentManager, GLEnums::SHADER_TYPE::COMPUTE, "lightCull.comp");
    if(!drawBinds.Init())
        return false;

    // Light count + light indices
    drawBinds["LightIndices"] = std::vector<int>(1 + workGroupCount.x * workGroupCount.y * maxLightsPerTile, 0);
    // offset + count + padding2
    drawBinds["TileLights"] = std::vector<int>(workGroupCount.x * workGroupCount.y* 4, -1);
    drawBinds["ScreenSize"] = glm::ivec2(screenWidth, screenHeight);

    glGenQueries(1, &timeQuery);

    return true;
}

void LightCull::Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse)
{
    PreDraw(viewMatrix, projectionMatrixInverse);

    glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);

    PostDraw();
}

GLuint64 LightCull::TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse)
{
    GLuint64 lightCullTime;

    PreDraw(viewMatrix, projectionMatrixInverse);

    glBeginQuery(GL_TIME_ELAPSED, timeQuery);
    glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);
    glEndQuery(GL_TIME_ELAPSED);

    GLint timeAvailable = 0;
    while(!timeAvailable)
        glGetQueryObjectiv(timeQuery,  GL_QUERY_RESULT_AVAILABLE, &timeAvailable);

    glGetQueryObjectui64v(timeQuery, GL_QUERY_RESULT, &lightCullTime);

    PostDraw();

    return lightCullTime;
}

void LightCull::DrawLightCount(SpriteRenderer& spriteRenderer, CharacterSet* characterSet)
{


    auto tileLightsSSBO = drawBinds.GetSSBO("TileLights");
    auto data = tileLightsSSBO->GetData();

    struct LightData
    {
        int start;
        int numberOfLights;
        glm::ivec2 padding;
    };
    LightData* intData = static_cast<LightData*>(data.get());

    int xSpacing = screenWidth / workGroupCount.x;
    int ySpacing = screenHeight / workGroupCount.y;

    for(int y = 0; y < workGroupCount.y; ++y)
    {
        for(int x = 0; x < workGroupCount.x; ++x)
        {
            int numberOfLights = intData[(workGroupCount.y - y - 1) * workGroupCount.x + x].numberOfLights;
            if(numberOfLights > 0)
            {
                if((x + y) % 2 == 0)
                    spriteRenderer.Draw(Rect(x * workGroupSize.x, y * workGroupSize.y, workGroupSize.x, workGroupSize.y), glm::vec4(1.0f, 1.0f, 1.0f, 0.1f));

                std::string lightCountText = std::to_string(numberOfLights);

                auto textWidth = characterSet->GetWidthAtIndex(lightCountText.c_str(), -1);

                spriteRenderer.Draw(Rect(x * xSpacing + xSpacing / 2 - textWidth / 2, y * ySpacing + ySpacing / 2 - characterSet->GetLineHeight() / 2, textWidth, characterSet->GetLineHeight()), glm::vec4(0.0f, 0.0f, 0.0f, 0.85f));
                spriteRenderer.DrawString(characterSet, lightCountText, glm::vec2(x * xSpacing + xSpacing / 2 - textWidth / 2, y * ySpacing + ySpacing / 2 - characterSet->GetLineHeight() / 2));
            }
        }
    }
}

void LightCull::PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse)
{
    int zero = 0;
    drawBinds.GetSSBO("LightIndices")->UpdateData(0, &zero, sizeof(int));

    drawBinds["viewMatrix"] = viewMatrix;
    drawBinds["projectionInverseMatrix"] = projectionMatrixInverse;

    drawBinds.Bind();
}

void LightCull::PostDraw()
{
    drawBinds.Unbind();
}

glm::uvec2 LightCull::GetWorkGroupSize() const
{
    return workGroupSize;
}

glm::uvec2 LightCull::GetThreadsPerGroup() const
{
    return threadsPerGroup;
}

glm::uvec2 LightCull::GetWorkGroupCount() const
{
    return workGroupCount;
}

int LightCull::GetMaxLightsPerTile() const
{
    return maxLightsPerTile;
}

void LightCull::ResolutionChanged(int newWidth, int newHeight)
{
    this->screenWidth = newWidth;
    this->screenHeight = newHeight;

    drawBinds["ScreenSize"] = glm::ivec2(newWidth, newHeight);

    std::vector<int> data((unsigned long)(newWidth * newHeight), -1);
    drawBinds["PixelToTile"] = data;

    workGroupCount.x = (GLuint)std::ceil(newWidth / (float)workGroupSize.x);
    workGroupCount.y = (GLuint)std::ceil(newHeight / (float)workGroupSize.y);
}
