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
                    std::make_pair("THREADS_PER_GROUP_X", "2")
                    , std::make_pair("THREADS_PER_GROUP_Y", "2")
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

    return true;
}

void LightCullAdaptive::Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse)
{
    PreDraw(viewMatrix, projectionMatrixInverse);

    Draw();

    PostDraw();
}

GLuint64 LightCullAdaptive::TimedDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse)
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

void LightCullAdaptive::PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse)
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

void LightCullAdaptive::DrawLightCount(SpriteRenderer& spriteRenderer
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

void LightCullAdaptive::SetDrawBindData(GLDrawBinds& binds)
{
    binds["Lights"] = lightCullDrawBinds["Lights"];
    binds["LightIndices"] = lightCullDrawBinds["LightIndices"];
    binds["TileLights"] = lightCullDrawBinds["TileLights"];
    binds["ScreenSize"] = lightCullDrawBinds["ScreenSize"];
    binds["Tree"] = lightCullDrawBinds["Tree"];
    binds["ReadWriteOffsets"] = lightCullDrawBinds["ReadWriteOffsets"];
    binds["TreeDepthData"] = lightCullDrawBinds["TreeDepthData"];
    binds["ColorBuffer"] = colors;
}

std::string LightCullAdaptive::GetForwardShaderPath()
{
    return "lightCullAdaptive/forward.frag";
}
