#include "lightCull.h"

#include "GL/gl3w.h"
#include "console/console.h"
#include "console/commandGetSet.h"

LightCull::LightCull()
        : threadsPerGroup(16, 16)
{}

LightCull::~LightCull()
{}

void LightCull::InitShaderConstants(int screenWidth, int screenHeight)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
}

std::vector<glm::vec4> tileColors;

bool LightCull::Init(ContentManager& contentManager, Console& console)
{
    console.AddCommand(new CommandGetSet<int>("treeMaxDepth", &treeMaxDepth));
    console.AddCommand(new CommandGetSet<int>("treeStartDepth", &treeStartDepth));

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

    if(MAX_LIGHTS_PER_TILE + 1 > maxSize / sizeof(int))
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
    lightCullDrawBinds["LightIndices"] = std::vector<int>(1 + GetMaxNumberOfTiles() * MAX_LIGHTS_PER_TILE * 2, -1);
    // start + numberOfLights + padding
    lightCullDrawBinds["TileLights"] = std::vector<int>(GetMaxNumberOfTiles() * MAX_LIGHTS_PER_TILE * 4 * 2, -1);
    lightCullDrawBinds["ScreenSize"] = glm::ivec2(screenWidth, screenHeight);
    lightCullDrawBinds["TreeDepthData"] = glm::ivec2(treeStartDepth, treeMaxDepth);

    ////////////////////////////////////////////////////////////
    // Light reduction

    lightReductionDrawBinds.AddUniform("viewMatrix", glm::mat4());
    lightReductionDrawBinds.AddUniform("projectionInverseMatrix", glm::mat4());

    lightReductionDrawBinds.AddUniform("oldDepth", 1);
    lightReductionDrawBinds.AddUniform("newDepth", 2);
    lightReductionDrawBinds.AddShaders(contentManager, GLEnums::SHADER_TYPE::COMPUTE, "lightReduction.comp");
    if(!lightReductionDrawBinds.Init())
        return false;

    // These are always the same
    lightReductionDrawBinds["ScreenSize"] = lightCullDrawBinds["ScreenSize"];
    lightReductionDrawBinds["Lights"] = lightCullDrawBinds["Lights"];
    lightReductionDrawBinds["LightIndices"] = lightCullDrawBinds["LightIndices"];
    lightReductionDrawBinds["TileLights"] = lightCullDrawBinds["TileLights"];
    lightReductionDrawBinds["Tree"] = lightCullDrawBinds["Tree"];
    lightReductionDrawBinds["ReadWriteOffsets"] = lightCullDrawBinds["ReadWriteOffsets"];
    lightReductionDrawBinds["TreeDepthData"] = lightCullDrawBinds["TreeDepthData"];

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
    ////////////////////////////////////////////////////////////
    // Light culling
    int zero = 0;
    //lightCullDrawBinds.GetSSBO("TileLights")->SetData(-1);
    //lightCullDrawBinds.GetSSBO("LightIndices")->SetData(-1);
    lightCullDrawBinds.GetSSBO("LightIndices")->UpdateData(0, &zero, sizeof(int));

    lightCullDrawBinds["viewMatrix"] = viewMatrix;
    lightCullDrawBinds["projectionInverseMatrix"] = projectionMatrixInverse;
    lightCullDrawBinds["TreeDepthData"] = glm::ivec2(treeStartDepth, treeMaxDepth);

    ////////////////////////////////////////////////////////////
    // Light reduction
    lightReductionDrawBinds["viewMatrix"] = viewMatrix;
    lightReductionDrawBinds["projectionInverseMatrix"] = projectionMatrixInverse;

    int size = 0;
    for(int i = 0; i < GetTreeMaxDepth(); ++i)
        size += pow(2, i + 1) * pow(2, i + 1);
    lightCullDrawBinds["Tree"] = std::vector<int>(size, -1);

    glm::ivec4 readWriteOffsets(0);
    lightCullDrawBinds["ReadWriteOffsets"] = readWriteOffsets;

    lightCullDrawBinds.Bind();
}

void LightCull::Draw()
{
    GLuint threadGroupCount = (GLuint)std::pow(2, treeStartDepth);

    glDispatchCompute(threadGroupCount, threadGroupCount, 1);

    lightCullDrawBinds.Unbind();

    GLsync writeSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    auto val = glClientWaitSync(writeSync, 0, 1000000000);
    if(val == GL_TIMEOUT_EXPIRED
       || val == GL_WAIT_FAILED)
    {
        assert(false);
    }
    glDeleteSync(writeSync);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);

    lightReductionDrawBinds.Bind();

    int indexLength = ((lightReductionDrawBinds.GetSSBO("LightIndices")->GetSize() - sizeof(int)) / sizeof(int)) / 2;
    int lightDataLength = (lightReductionDrawBinds.GetSSBO("TileLights")->GetSize() / (sizeof(int) * 4)) / 2;

    glm::ivec4 readWriteOffsets(0);
    for(int depth = treeStartDepth + 1; depth <= treeMaxDepth; ++depth)
    {
        lightReductionDrawBinds["oldDepth"] = depth - 1;
        lightReductionDrawBinds["newDepth"] = depth;

        readWriteOffsets.x = depth % 2 * indexLength;
        readWriteOffsets.y = (depth + 1) % 2 * indexLength;
        readWriteOffsets.z = depth % 2 * lightDataLength;
        readWriteOffsets.w = (depth + 1) % 2 * lightDataLength;
        lightCullDrawBinds["ReadWriteOffsets"] = readWriteOffsets;
        lightCullDrawBinds.GetUBO("ReadWriteOffsets")->Update();

        threadGroupCount = (GLuint)std::pow(2, depth);

        glDispatchCompute(threadGroupCount, threadGroupCount, 1);

        writeSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        val = glClientWaitSync(writeSync, 0, 1000000000);
        if(val == GL_TIMEOUT_EXPIRED
            || val == GL_WAIT_FAILED)
        {
            assert(false);
        }
        glDeleteSync(writeSync);
        glMemoryBarrier(GL_ALL_BARRIER_BITS);
    }
}

void LightCull::PostDraw()
{
    lightReductionDrawBinds.Unbind();
}

int GetTreeLinearIndex(int x, int y)
{
    int index = 0;

    for(int i = 0; i < 32; ++i)
    {
        int extractedBit;

        if(i % 2 == 0)
            extractedBit = ((x >> (i / 2)) & 1);
        else
            extractedBit = (y >> (i / 2)) & 1;

        index |= (extractedBit << i);
    }

    return index;
}

int LightCull::GetTreeDataScreen(int screenX, int screenY, int* tree)
{
    int lastIndex = -1;

    int depthOffset = -1;

    glm::vec2 range = glm::vec2(float(screenWidth), float(screenHeight));

    for(int i = 0; i < treeMaxDepth; ++i)
    {
        depthOffset += int(pow(4, i));

        range /= 2.0f;

        int x = int(screenX / range.x);
        int y = int(screenY / range.y);

        int index = GetTreeLinearIndex(x, y);

        int potentialIndex = tree[depthOffset + index];
        if(potentialIndex < 0)
            return i;
        else
            lastIndex = i;
    }

    return lastIndex;
}

void LightCull::DrawLightCount(SpriteRenderer& spriteRenderer
                               , CharacterSet* characterSetSmall
                               , CharacterSet* characterSetBig)
{
    auto tree = lightCullDrawBinds.GetSSBO("Tree")->GetData();

    int startOffset = -1;
    for(int i = 0; i < treeMaxDepth; ++i)
        startOffset += std::pow(2, i) * std::pow(2, i);

    int threadCount = (int)(std::pow(2, treeMaxDepth));
    int spacing = screenWidth / threadCount;

    for(int y = 0; y < threadCount; ++y)
    {
        for(int x = 0; x < threadCount; ++x)
        {
            int data = GetTreeDataScreen(x * spacing, (threadCount - y - 1) * spacing, (int*)tree.get());

            //spriteRenderer.DrawString(characterSet, std::to_string(((int*)tree.get())[startOffset + index]), glm::vec2(spacing * x, spacing * y));

            std::string text = std::to_string(data);
            auto textWidth = characterSetSmall->GetWidthAtIndex(text.c_str(), -1);

            glm::vec2 drawPosition = glm::vec2(x * spacing + spacing / 2 - textWidth / 2, y * spacing + spacing / 2 - characterSetSmall->GetLineHeight() / 2);
            drawPosition = glm::clamp(drawPosition, glm::vec2(0.0f), glm::vec2(screenWidth - textWidth, screenHeight - characterSetSmall->GetLineHeight()));

            spriteRenderer.Draw(Rect(drawPosition, textWidth, characterSetSmall->GetLineHeight()), glm::vec4(0.0f, 0.0f, 0.0f, 0.85f));
            spriteRenderer.DrawString(characterSetSmall, text, drawPosition, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        }
    }

    //return;

    //auto tileLightsSSBO = lightCullDrawBinds.GetSSBO("TileLights");
    /*auto tileLightsSSBO = lightReductionDrawBinds.GetSSBO("NewTileLights");
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

    float maxWidth = characterSet->GetWidthAtMaxWidth(std::to_string(MAX_LIGHTS_PER_TILE).c_str(), -1);
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

            float color = numberOfLights / (float)MAX_LIGHTS_PER_TILE;
            spriteRenderer.Draw(Rect(x, y, 1.0f, 1.0f), tileColors[index]);


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
            }
        }
    }*/
}

glm::uvec2 LightCull::GetThreadsPerGroup() const
{
    return threadsPerGroup;
}

int LightCull::GetMaxLightsPerTile() const
{
    return MAX_LIGHTS_PER_TILE;
}

int LightCull::GetMaxNumberOfTreeIndices() const
{
    int size = 0;
    for(int i = 1; i <= TREE_MAX_DEPTH; ++i) // TODO: + 1?
        size += (int)(std::pow(2, i) * std::pow(2, i));

    return size;
}

int LightCull::GetMaxNumberOfTiles() const
{
    return (int)(std::pow(2, TREE_MAX_DEPTH) * std::pow(2, TREE_MAX_DEPTH));
}

int LightCull::GetTreeStartDepth() const
{
    return treeStartDepth;
}

int LightCull::GetTreeMaxDepth() const
{
    return TREE_MAX_DEPTH;
}

void LightCull::ResolutionChanged(int newWidth, int newHeight)
{
    this->screenWidth = newWidth;
    this->screenHeight = newHeight;

    lightCullDrawBinds["ScreenSize"] = glm::ivec2(newWidth, newHeight);

    std::vector<int> data((unsigned long)(newWidth * newHeight), -1);
    //lightCullDrawBinds["PixelToTile"] = data;gh
}

/*GLShaderStorageBuffer* LightCull::GetActiveTileLightsData()
{
    return tileLightData[activeTileLightsData].get();
}*/
