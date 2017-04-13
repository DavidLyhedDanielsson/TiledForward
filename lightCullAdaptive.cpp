#include "lightCullAdaptive.h"

#include "GL/gl3w.h"
#include "console/console.h"
#include "console/commandGetSet.h"
#include "gl/glCPPShared.h"

LightCullAdaptive::LightCullAdaptive()
{}

LightCullAdaptive::~LightCullAdaptive()
{}

void LightCullAdaptive::InitShaderConstants(int screenWidth, int screenHeight)
{
    LightCull::InitShaderConstants(screenWidth, screenHeight);
}

bool LightCullAdaptive::Init(ContentManager& contentManager, Console& console)
{
    for(int i = 0; i < GetMaxNumberOfTreeIndices(); ++i)
        colors.push_back({rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), 1.0f});

    GLCPPSharedContentParameters sharedParameters;
    sharedParameters.variables =
            {
                    std::make_pair("THREADS_PER_GROUP_X", "16")
                    , std::make_pair("THREADS_PER_GROUP_Y", "16")
                    , std::make_pair("MAX_LIGHTS_PER_TILE", std::to_string(GetMaxLightsPerTile()))
            };
    sharedParameters.outPath = std::string(contentManager.GetRootDir()) + "/lightCullAdaptive";
    sharedVariables = contentManager.Load<GLCPPShared>("lightCullAdaptive/shared.h", &sharedParameters);

    console.AddCommand(new CommandGetSet<int>("treeMaxDepth", &treeMaxDepth));
    console.AddCommand(new CommandGetSet<int>("treeStartDepth", &treeStartDepth));

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
    lightCullDrawBinds.AddShaders(contentManager, GLEnums::SHADER_TYPE::COMPUTE, "lightCullAdaptive/lightCull.comp");
    if(!lightCullDrawBinds.Init())
        return false;

    // Light count + light indices
    lightCullDrawBinds["LightIndices"] = std::vector<int>(GetMaxNumberOfTiles() * MAX_LIGHTS_PER_TILE * 2, -1);
    // start + numberOfLights + padding
    lightCullDrawBinds["TileLights"] = std::vector<int>(GetMaxNumberOfTiles() * 4 * 2, -1);
    lightCullDrawBinds["ScreenSize"] = glm::ivec2(screenWidth, screenHeight);
    lightCullDrawBinds["TreeDepthData"] = glm::ivec2(treeStartDepth, treeMaxDepth);

    ////////////////////////////////////////////////////////////
    // Light reduction

    //lightReductionDrawBinds.AddUniform("viewMatrix", glm::mat4());
    lightReductionDrawBinds.AddUniform("projectionInverseMatrix", glm::mat4());

    lightReductionDrawBinds.AddUniform("oldDepth", 1);
    lightReductionDrawBinds.AddUniform("newDepth", 2);
    lightReductionDrawBinds.AddShaders(contentManager, GLEnums::SHADER_TYPE::COMPUTE, "lightCullAdaptive/lightReduction.comp");
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

    // Normal draw binds
    if(!forwardDrawBinds.AddShaders(contentManager
                             , GLEnums::SHADER_TYPE::VERTEX, "lightCullAdaptive/forward.vert"
                             , GLEnums::SHADER_TYPE::FRAGMENT, "lightCullAdaptive/forward.frag"))
        return false;

    forwardDrawBinds.AddUniform("projectionMatrix", glm::mat4x4());
    forwardDrawBinds.AddUniform("worldViewMatrix", glm::mat4x4());

    return true;
}

void LightCullAdaptive::Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager)
{
    PreDraw(viewMatrix, projectionMatrixInverse, lightManager);

    Draw();

    PostDraw();
}

GLuint64 LightCullAdaptive::TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager)
{
    GLuint64 lightCullTime;

    PreDraw(viewMatrix, projectionMatrixInverse, lightManager);

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

void LightCullAdaptive::PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager)
{
    ////////////////////////////////////////////////////////////
    // Light culling
    int zero = 0;
    lightCullDrawBinds.GetSSBO("TileLights")->SetData(-1);
    //lightCullDrawBinds.GetSSBO("LightIndices")->SetData(-1);
    //lightCullDrawBinds.GetSSBO("LightIndices")->UpdateData(0, &zero, sizeof(int));

    lightCullDrawBinds["Lights"] = &lightManager.GetLightsBuffer();
    lightCullDrawBinds["viewMatrix"] = viewMatrix;
    lightCullDrawBinds["projectionInverseMatrix"] = projectionMatrixInverse;
    lightCullDrawBinds["TreeDepthData"] = glm::ivec2(treeStartDepth, treeMaxDepth);

    ////////////////////////////////////////////////////////////
    // Light reduction
    //lightReductionDrawBinds["viewMatrix"] = viewMatrix;
    lightReductionDrawBinds["projectionInverseMatrix"] = projectionMatrixInverse;

    int size = 0;
    for(int i = 0; i < GetTreeMaxDepth(); ++i)
        size += pow(2, i + 1) * pow(2, i + 1);
    lightCullDrawBinds["Tree"] = std::vector<int>(size, -1);

    glm::ivec4 readWriteOffsets(0);
    lightCullDrawBinds["ReadWriteOffsets"] = readWriteOffsets;

    lightCullDrawBinds.Bind();
}

void LightCullAdaptive::Draw()
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

    int indexLength = ((lightReductionDrawBinds.GetSSBO("LightIndices")->GetSize()) / sizeof(int)) / 2;
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

    readWriteOffsets.x = (treeMaxDepth + 1) % 2 * indexLength;
    readWriteOffsets.y = (treeMaxDepth + 2) % 2 * indexLength;
    readWriteOffsets.z = (treeMaxDepth + 1) % 2 * lightDataLength;
    readWriteOffsets.w = (treeMaxDepth + 2) % 2 * lightDataLength;
    lightCullDrawBinds["ReadWriteOffsets"] = readWriteOffsets;
    lightCullDrawBinds.GetUBO("ReadWriteOffsets")->Update();
}

void LightCullAdaptive::PostDraw()
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

int LightCullAdaptive::GetTreeDataScreen(int screenX, int screenY, int* tree)
{
    int depthOffset = 0;

    for(int i = 1; i < treeMaxDepth; ++i)
        depthOffset += int(pow(4, i));

    int index = GetTreeLinearIndex(screenX, screenY);

    return tree[depthOffset + index];
}

void LightCullAdaptive::DrawLightCount(SpriteRenderer& spriteRenderer
                               , CharacterSet* characterSetSmall
                               , CharacterSet* characterSetBig)
{
    auto tree = lightCullDrawBinds.GetSSBO("Tree")->GetData();

    struct TileLight
    {
        int start;
        int count;
        int padding0;
        int padding1;
    };

    int indexLength = ((lightReductionDrawBinds.GetSSBO("LightIndices")->GetSize()) / sizeof(int)) / 2;
    int lightDataLength = (lightReductionDrawBinds.GetSSBO("TileLights")->GetSize() / (sizeof(int) * 4)) / 2;
    glm::ivec4 readWriteOffsets;
    readWriteOffsets.x = (treeMaxDepth + 1) % 2 * indexLength;
    readWriteOffsets.y = (treeMaxDepth + 2) % 2 * indexLength;
    readWriteOffsets.z = (treeMaxDepth + 1) % 2 * lightDataLength;
    readWriteOffsets.w = (treeMaxDepth + 2) % 2 * lightDataLength;

    auto tileLights = lightCullDrawBinds.GetSSBO("TileLights")->GetData();

    int threadCount = (int)(std::pow(2, treeMaxDepth));
    int spacing = screenWidth / threadCount;

    for(int y = 0; y < threadCount; ++y)
    {
        for(int x = 0; x < threadCount; ++x)
        {
            int treeData = GetTreeDataScreen(x, (threadCount - y - 1), (int*)tree.get());

            if(treeData >= 0)
            {
                TileLight currentTile = ((TileLight*)tileLights.get())[readWriteOffsets.z + treeData];

                //spriteRenderer.DrawString(characterSet, std::to_string(((int*)tree.get())[startOffset + index]), glm::vec2(spacing * x, spacing * y));

                std::string text = std::to_string(currentTile.count);
                auto textWidth = characterSetSmall->GetWidthAtIndex(text.c_str(), -1);

                glm::vec2 drawPosition = glm::vec2(x * spacing + spacing / 2 - textWidth / 2, y * spacing + spacing / 2 - characterSetSmall->GetLineHeight() / 2);
                drawPosition = glm::clamp(drawPosition, glm::vec2(0.0f), glm::vec2(screenWidth - textWidth, screenHeight - characterSetSmall->GetLineHeight()));

                spriteRenderer.Draw(Rect(drawPosition, textWidth, characterSetSmall->GetLineHeight()), glm::vec4(0.0f, 0.0f, 0.0f, 0.85f));

                glm::vec4 textColor;

                if(currentTile.count > MAX_LIGHTS_PER_TILE)
                    textColor = { 1.0f, 0.0f, 0.0f, 1.0f };
                else if(currentTile.count == MAX_LIGHTS_PER_TILE)
                    textColor = { 0.0f, 0.0f, 1.0f, 1.0f };
                else
                    textColor = { 1.0f, 1.0f, 1.0f, 1.0f };

                spriteRenderer.DrawString(characterSetSmall, text, drawPosition, textColor);
            }
        }
    }
}

int LightCullAdaptive::GetMaxLightsPerTile() const
{
    return MAX_LIGHTS_PER_TILE;
}

int LightCullAdaptive::GetMaxNumberOfTreeIndices() const
{
    int size = 0;
    for(int i = 1; i <= TREE_MAX_DEPTH; ++i) // TODO: + 1?
        size += (int)(std::pow(2, i) * std::pow(2, i));

    return size;
}

int LightCullAdaptive::GetMaxNumberOfTiles() const
{
    return (int)(std::pow(2, TREE_MAX_DEPTH) * std::pow(2, TREE_MAX_DEPTH));
}

int LightCullAdaptive::GetTreeStartDepth() const
{
    return treeStartDepth;
}

int LightCullAdaptive::GetTreeMaxDepth() const
{
    return TREE_MAX_DEPTH;
}

void LightCullAdaptive::ResolutionChanged(int newWidth, int newHeight)
{
    this->screenWidth = newWidth;
    this->screenHeight = newHeight;

    lightCullDrawBinds["ScreenSize"] = glm::ivec2(newWidth, newHeight);

    std::vector<int> data((unsigned long)(newWidth * newHeight), -1);
}

void LightCullAdaptive::SetDrawBindData()
{
    forwardDrawBinds["Lights"] = lightCullDrawBinds["Lights"];
    forwardDrawBinds["LightIndices"] = lightCullDrawBinds["LightIndices"];
    forwardDrawBinds["TileLights"] = lightCullDrawBinds["TileLights"];
    forwardDrawBinds["ScreenSize"] = lightCullDrawBinds["ScreenSize"];
    forwardDrawBinds["Tree"] = lightCullDrawBinds["Tree"];
    forwardDrawBinds["ReadWriteOffsets"] = lightCullDrawBinds["ReadWriteOffsets"];
    forwardDrawBinds["TreeDepthData"] = lightCullDrawBinds["TreeDepthData"];
    forwardDrawBinds["ColorBuffer"] = colors;
}

GLDrawBinds* LightCullAdaptive::GetForwardDrawBinds()
{
    return &forwardDrawBinds;
}

void LightCullAdaptive::UpdateUniforms(PerspectiveCamera* currentCamera, OBJModel* worldModel, LightManager* lightManager)
{
    auto viewMatrix = currentCamera->GetViewMatrix();
    auto viewMatrixInverse = glm::inverse(currentCamera->GetViewMatrix());
    auto projectionMatrix = currentCamera->GetProjectionMatrix();
    auto projectionMatrixInverse = glm::inverse(currentCamera->GetProjectionMatrix());
    auto viewProjectionMatrix = projectionMatrix * viewMatrix;
    auto worldViewMatrix = viewMatrix * worldModel->worldMatrix;

    forwardDrawBinds["projectionMatrix"] = projectionMatrix;
    forwardDrawBinds["worldViewMatrix"] = worldViewMatrix;
    forwardDrawBinds["Lights"] = &lightManager->GetLightsBuffer();
}

/*std::string LightCullAdaptive::GetForwardShaderPath()
{
    return "lightCullAdaptive/forward.frag";
}

std::string LightCullAdaptive::GetForwardShaderDebugPath()
{
    return "lightCullAdaptive/forwardDebug.frag";
}*/
