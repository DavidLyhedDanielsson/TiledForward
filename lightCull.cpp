#include "lightCull.h"

#include "GL/gl3w.h"

LightCull::LightCull()
        : workGroupSize(64, 64)
          , threadsPerGroup(16, 16)
          , maxLightsPerTile(1)
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

std::vector<glm::vec4> tileColors;

bool LightCull::Init(ContentManager& contentManager)
{
    for(int i = 0; i < screenWidth * screenHeight; ++i)
    {
        float r = rand() / (float)RAND_MAX;
        float g = rand() / (float)RAND_MAX;
        float b = rand() / (float)RAND_MAX;

        tileColors.push_back(glm::vec4(r, g, b, 0.5f));
    }

    ////////////////////////////////////////////////////////////
    // Make sure there aren't too many lights per tile

    GLint maxSize;
    glGetIntegerv(GL_MAX_COMPUTE_SHARED_MEMORY_SIZE, &maxSize);

    if(maxLightsPerTile + 1 > maxSize / sizeof(int))
    {
        Logger::LogLine(LOG_TYPE::FATAL, "GPU only supports a maximum of ", maxSize / sizeof(int), " ints in shared memory");
        return false;
    }

    ////////////////////////////////////////////////////////////
    // Light culling

    lightCullDrawBinds.AddUniform("viewMatrix", glm::mat4());
    lightCullDrawBinds.AddUniform("projectionInverseMatrix", glm::mat4());
    lightCullDrawBinds.AddShaders(contentManager, GLEnums::SHADER_TYPE::COMPUTE, "lightCull.comp");
    if(!lightCullDrawBinds.Init())
        return false;

    // Light count + light indices
    lightCullDrawBinds["LightIndices"] = std::vector<int>(1 + workGroupCount.x * workGroupCount.y * maxLightsPerTile, 0);
    // offset + count + padding2
    lightCullDrawBinds["TileLights"] = std::vector<int>(workGroupCount.x * workGroupCount.y * 4, -1);
    lightCullDrawBinds["ScreenSize"] = glm::ivec2(screenWidth, screenHeight);

    ////////////////////////////////////////////////////////////
    // Light reduction

    lightReductionDrawBinds.AddUniform("viewMatrix", glm::mat4());
    lightReductionDrawBinds.AddUniform("projectionInverseMatrix", glm::mat4());
    lightReductionDrawBinds.AddUniform("workGroupCountX", (int)workGroupCount.x);

    lightReductionDrawBinds.AddUniform("oldWorkGroupCountX", (int)workGroupCount.x);
    lightReductionDrawBinds.AddUniform("oldWorkGroupCountY", (int)workGroupCount.y);
    lightReductionDrawBinds.AddUniform("newWorkGroupCountX", (int)workGroupCount.x * 2);
    lightReductionDrawBinds.AddUniform("newWorkGroupCountY", (int)workGroupCount.y * 2);
    lightReductionDrawBinds.AddUniform("oldTileSizeX", (int)workGroupSize.x);
    lightReductionDrawBinds.AddUniform("oldTileSizeY", (int)workGroupSize.y);
    lightReductionDrawBinds.AddUniform("newTileSizeX", (int)workGroupSize.x / 2);
    lightReductionDrawBinds.AddUniform("newTileSizeY", (int)workGroupSize.y / 2);
    lightReductionDrawBinds.AddShaders(contentManager, GLEnums::SHADER_TYPE::COMPUTE, "lightReduction.comp");
    if(!lightReductionDrawBinds.Init())
        return false;

    // These are always the same
    lightReductionDrawBinds["ScreenSize"] = lightCullDrawBinds["ScreenSize"];
    lightReductionDrawBinds["Lights"] = lightCullDrawBinds["Lights"];

    //These aren't
    // Light count + light indices * 4 (for reduction)
    lightReductionDrawBinds["NewLightIndices"] = std::vector<int>(1 + (workGroupCount.x * workGroupCount.y * 4) * maxLightsPerTile, 0);
    // offset + count + padding2
    lightReductionDrawBinds["NewTileLights"] = std::vector<int>((workGroupCount.x * workGroupCount.y * 4) * 4, -1);

    lightReductionDrawBinds["OldLightIndices"] = lightCullDrawBinds["LightIndices"];
    lightReductionDrawBinds["OldTileLights"] = lightCullDrawBinds["TileLights"];
    lightReductionDrawBinds["OldPixelToTile"] = lightCullDrawBinds["PixelToTile"];

    glGenQueries(1, &timeQuery);

    return true;
}

void LightCull::Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse)
{
    PreDraw(viewMatrix, projectionMatrixInverse);

    Draw();

    PostDraw();
}

GLuint64 LightCull::TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse)
{
    GLuint64 lightCullTime;

    PreDraw(viewMatrix, projectionMatrixInverse);

    glBeginQuery(GL_TIME_ELAPSED, timeQuery);
    Draw();
    glEndQuery(GL_TIME_ELAPSED);

    GLint timeAvailable = 0;
    while(!timeAvailable)
        glGetQueryObjectiv(timeQuery,  GL_QUERY_RESULT_AVAILABLE, &timeAvailable);

    glGetQueryObjectui64v(timeQuery, GL_QUERY_RESULT, &lightCullTime);

    PostDraw();

    return lightCullTime;
}

void LightCull::PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse)
{
    int zero = 0;
    ////////////////////////////////////////////////////////////
    // Light culling
    lightCullDrawBinds.GetSSBO("LightIndices")->UpdateData(0, &zero, sizeof(int));

    lightCullDrawBinds["viewMatrix"] = viewMatrix;
    lightCullDrawBinds["projectionInverseMatrix"] = projectionMatrixInverse;

    ////////////////////////////////////////////////////////////
    // Light reduction
    lightReductionDrawBinds.GetSSBO("NewLightIndices")->UpdateData(0, &zero, sizeof(int));
    //lightReductionDrawBinds.GetSSBO("CurrentTileLights")->SetData(-1);

    lightReductionDrawBinds["viewMatrix"] = viewMatrix;
    lightReductionDrawBinds["projectionInverseMatrix"] = projectionMatrixInverse;

    lightCullDrawBinds.Bind();
}

void LightCull::Draw()
{
    glDispatchCompute(workGroupCount.x, workGroupCount.y, 1);

    lightCullDrawBinds.Unbind();

    //GLsync writeSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    //glClientWaitSync(writeSync, 0, 1000000000);
    //glDeleteSync(writeSync);

    /*lightReductionDrawBinds.Bind();

    glDispatchCompute(workGroupCount.x * 2, workGroupCount.y * 2, 1);

    writeSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    glClientWaitSync(writeSync, 0, 1000000000);
    glDeleteSync(writeSync);*/
}

void LightCull::PostDraw()
{
    //lightReductionDrawBinds.Unbind();
}

void LightCull::DrawLightCount(SpriteRenderer& spriteRenderer, CharacterSet* characterSet)
{
    return;

    //auto tileLightsSSBO = lightCullDrawBinds.GetSSBO("TileLights");
    auto tileLightsSSBO = lightReductionDrawBinds.GetSSBO("NewTileLights");
    auto data = tileLightsSSBO->GetData();

    struct LightData
    {
        int start;
        int numberOfLights;
        glm::ivec2 padding;
    };
    LightData* intData = static_cast<LightData*>(data.get());

    int xSpacing = workGroupSize.x;
    int ySpacing = workGroupSize.y;

    float maxWidth = characterSet->GetWidthAtMaxWidth(std::to_string(maxLightsPerTile).c_str(), -1);
    float maxHeight = characterSet->GetLineHeight();

    //auto pixelToTile = lightCullDrawBinds.GetSSBO("PixelToTile")->GetData();
    auto pixelToTile = lightReductionDrawBinds.GetSSBO("NewPixelToTile")->GetData();

    //for(int y = 0; y < workGroupCount.y; ++y)
    for(int y = 0; y < 720; ++y)
    {
        //for(int x = 0; x < workGroupCount.x; ++x)
        for(int x = 0; x < 1280; ++x)
        {
            //int pixelIndex = ((workGroupCount.y - y - 1) * workGroupCount.x * workGroupSize.x * workGroupSize.y) + (x * workGroupSize.x);
            int pixelIndex = (720 - y - 1) * 1280 + x;
            int index = static_cast<int*>(pixelToTile.get())[pixelIndex];

            //assert(index >= 0 && index < screenWidth * screenHeight);

            int numberOfLights = intData[index].numberOfLights;

            float color = numberOfLights / (float)maxLightsPerTile;
            spriteRenderer.Draw(Rect(x, y, 1.0f, 1.0f), tileColors[index]);

            /*
            if(numberOfLights != 0)
            {
                if((x + y) % 2 == 0)
                    spriteRenderer.Draw(Rect(x * workGroupSize.x, y * workGroupSize.y, workGroupSize.x, workGroupSize.y), glm::vec4(1.0f, 1.0f, 1.0f, 0.1f));

                std::string lightCountText = std::to_string(numberOfLights);

                auto textWidth = characterSet->GetWidthAtIndex(lightCountText.c_str(), -1);

                if(workGroupSize.x > maxWidth && workGroupSize.y > maxHeight)
                {
                    glm::vec2 drawPosition = glm::vec2(x * xSpacing + xSpacing / 2 - textWidth / 2, y * ySpacing + ySpacing / 2 - characterSet->GetLineHeight() / 2);
                    drawPosition = glm::clamp(drawPosition, glm::vec2(0.0f), glm::vec2(screenWidth - textWidth, screenHeight - characterSet->GetLineHeight()));

                    spriteRenderer.Draw(Rect(drawPosition, textWidth, characterSet->GetLineHeight()), glm::vec4(0.0f, 0.0f, 0.0f, 0.85f));
                    spriteRenderer.DrawString(characterSet, lightCountText, drawPosition, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
            }*/
        }
    }

    /*auto oldData = std::move(data);
    auto oldIntData = intData;

    tileLightsSSBO = lightReductionDrawBinds.GetSSBO("NewTileLights");
    data = tileLightsSSBO->GetData();

    intData = static_cast<LightData*>(data.get());

    glm::uvec2 tempSize = glm::uvec2(workGroupSize.x / 2, workGroupSize.y / 2);
    glm::uvec2 tempCount = glm::uvec2(workGroupCount.x * 2, workGroupCount.y * 2);

    xSpacing = tempSize.x;
    ySpacing = tempSize.y;

    for(int y = 0; y < tempCount.y; ++y)
    {
        for(int x = 0; x < tempCount.x; ++x)
        {
            int numberOfLights = intData[(tempCount.y - y - 1) * tempCount.x + x].numberOfLights;
            int oldNumberOfLights = oldIntData[(workGroupCount.y - y / 2 - 1) * workGroupCount.x + x / 2].numberOfLights;
            if(numberOfLights != oldNumberOfLights)
            {
                std::string lightCountText = std::to_string(numberOfLights);

                auto textWidth = characterSet->GetWidthAtIndex(lightCountText.c_str(), -1);

                glm::vec2 drawPosition = glm::vec2(x * xSpacing + xSpacing / 2 - textWidth / 2, y * ySpacing + ySpacing / 2 - characterSet->GetLineHeight() / 2);
                drawPosition = glm::clamp(drawPosition, glm::vec2(0.0f), glm::vec2(screenWidth - textWidth, screenHeight - characterSet->GetLineHeight()));

                spriteRenderer.Draw(Rect(drawPosition, textWidth, characterSet->GetLineHeight()), glm::vec4(1.0f, 0.0f, 0.0f, 0.85f));
                spriteRenderer.DrawString(characterSet, lightCountText, drawPosition);

            }
        }
    }*/
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

    lightCullDrawBinds["ScreenSize"] = glm::ivec2(newWidth, newHeight);

    std::vector<int> data((unsigned long)(newWidth * newHeight), -1);
    lightCullDrawBinds["PixelToTile"] = data;
    lightReductionDrawBinds["NewPixelToTile"] = data;

    workGroupCount.x = (GLuint)std::ceil(newWidth / (float)workGroupSize.x);
    workGroupCount.y = (GLuint)std::ceil(newHeight / (float)workGroupSize.y);
}
