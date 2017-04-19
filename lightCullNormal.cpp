#include "lightCullNormal.h"

LightCullNormal::LightCullNormal()
        : threadsPerGroup(16, 16)
{}

LightCullNormal::~LightCullNormal()
{}

void LightCullNormal::InitShaderConstants(int screenWidth, int screenHeight)
{
    LightCull::InitShaderConstants(screenWidth, screenHeight);
}

bool LightCullNormal::Init(ContentManager& contentManager, Console& console)
{
    threadGroupCount = glm::ivec2((int)std::pow(2, MAX_DEPTH));

    for(int i = 0; i < GetMaxNumberOfTiles(); ++i)
        colors.push_back({rand() / float(RAND_MAX), rand() / float(RAND_MAX), rand() / float(RAND_MAX), 1.0f});

    GLCPPSharedContentParameters sharedParameters;
    sharedParameters.variables =
            {
                    std::make_pair("THREADS_PER_GROUP_X", std::to_string(GetThreadsPerGroup().x))
                    , std::make_pair("THREADS_PER_GROUP_Y", std::to_string(GetThreadsPerGroup().y))
                    , std::make_pair("MAX_LIGHTS_PER_TILE", std::to_string(GetMaxLightsPerTile()))
                    , std::make_pair("THREAD_GROUP_SIZE_X", std::to_string(screenWidth / threadGroupCount.x))
                    , std::make_pair("THREAD_GROUP_SIZE_Y", std::to_string(screenHeight / threadGroupCount.y))
                    , std::make_pair("THREAD_GROUP_COUNT_X", std::to_string(threadGroupCount.x))
                    , std::make_pair("THREAD_GROUP_COUNT_Y", std::to_string(threadGroupCount.y))
            };
    sharedParameters.outPath = std::string(contentManager.GetRootDir()) + "/lightCullNormal";
    sharedVariables = contentManager.Load<GLCPPShared>("lightCullNormal/shared.h", &sharedParameters);

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
    lightCullDrawBinds.AddShaders(contentManager, GLEnums::SHADER_TYPE::COMPUTE, "lightCullNormal/lightCull.comp");
    if(!lightCullDrawBinds.Init())
        return false;

    // Light count + light indices
    lightCullDrawBinds["LightIndices"] = std::vector<int>((unsigned long)(1 + GetMaxNumberOfTiles() * MAX_LIGHTS_PER_TILE)
                                                          , -1);
    // start + numberOfLights + padding
    lightCullDrawBinds["TileLights"] = std::vector<int>((unsigned long)(GetMaxNumberOfTiles() * MAX_LIGHTS_PER_TILE * 4)
                                                        , -1);
    lightCullDrawBinds["ScreenSize"] = glm::ivec2(screenWidth, screenHeight);

    // Normal draw binds
    if(!forwardDrawBinds.AddShaders(contentManager
                                    , GLEnums::SHADER_TYPE::VERTEX, "lightCullNormal/forward.vert"
                                    , GLEnums::SHADER_TYPE::FRAGMENT, "lightCullNormal/forward.frag"))
        return false;

    forwardDrawBinds.AddUniform("viewProjectionMatrix", glm::mat4x4());
    forwardDrawBinds.AddUniform("worldMatrix", glm::mat4x4());
    forwardDrawBinds.AddUniform("materialIndex", 0);

    return true;
}

void LightCullNormal::Draw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager)
{
    PreDraw(viewMatrix, projectionMatrixInverse, lightManager);

    std::vector<std::pair<std::string, GLuint64>> stuff;
    Draw(stuff);

    PostDraw();
}

std::vector<std::pair<std::string, GLuint64>> LightCullNormal::TimedDraw(glm::mat4 viewMatrix
                                                            , glm::mat4 projectionMatrixInverse
                                                            , LightManager& lightManager)
{
    std::vector<std::pair<std::string, GLuint64>> returnStuff;

    PreDraw(viewMatrix, projectionMatrixInverse, lightManager);

    Draw(returnStuff);

    PostDraw();

    return returnStuff;
}

void LightCullNormal::PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager)
{
    ////////////////////////////////////////////////////////////
    // Light culling
    int zero = 0;
    lightCullDrawBinds.GetSSBO("LightIndices")->UpdateData(0, &zero, sizeof(int));

    lightCullDrawBinds["Lights"] = &lightManager.GetLightsBuffer();
    lightCullDrawBinds["viewMatrix"] = viewMatrix;
    lightCullDrawBinds["projectionInverseMatrix"] = projectionMatrixInverse;

    lightCullDrawBinds.Bind();
}

void LightCullNormal::Draw(std::vector<std::pair<std::string, GLuint64>>& times)
{
    StartTimeQuery();
    glDispatchCompute((GLuint)threadGroupCount.x, (GLuint)threadGroupCount.y, 1);
    times.push_back(std::make_pair("Light cull", StopTimeQuery()));

    lightCullDrawBinds.Unbind();
}

void LightCullNormal::PostDraw()
{
    lightCullDrawBinds.Unbind();
}

glm::uvec2 LightCullNormal::GetThreadsPerGroup() const
{
    return threadsPerGroup;
}

int LightCullNormal::GetMaxLightsPerTile() const
{
    return MAX_LIGHTS_PER_TILE;
}

void LightCullNormal::ResolutionChanged(int newWidth, int newHeight)
{
    this->screenWidth = newWidth;
    this->screenHeight = newHeight;

    lightCullDrawBinds["ScreenSize"] = glm::ivec2(newWidth, newHeight);

    std::vector<int> data((unsigned long)(newWidth * newHeight), -1);
    //lightCullDrawBinds["PixelToTile"] = data;gh
}

void LightCullNormal::SetDrawBindData()
{
    forwardDrawBinds["Lights"] = lightCullDrawBinds["Lights"];
    forwardDrawBinds["LightIndices"] = lightCullDrawBinds["LightIndices"];
    forwardDrawBinds["TileLights"] = lightCullDrawBinds["TileLights"];
    forwardDrawBinds["ScreenSize"] = lightCullDrawBinds["ScreenSize"];
    forwardDrawBinds["ColorBuffer"] = colors;
}

int LightCullNormal::GetMaxNumberOfTiles() const
{
    return threadGroupCount.x * threadGroupCount.y;
}

void LightCullNormal::DrawLightCount(SpriteRenderer& spriteRenderer
                                     , CharacterSet* characterSetSmall
                                     , CharacterSet* characterSetBig)
{

}

GLDrawBinds* LightCullNormal::GetForwardDrawBinds()
{
    return &forwardDrawBinds;
}

void LightCullNormal::UpdateUniforms(PerspectiveCamera* currentCamera, OBJModel* worldModel, LightManager* lightManager)
{
    auto viewMatrix = currentCamera->GetViewMatrix();
    auto viewMatrixInverse = glm::inverse(currentCamera->GetViewMatrix());
    auto projectionMatrix = currentCamera->GetProjectionMatrix();
    auto projectionMatrixInverse = glm::inverse(currentCamera->GetProjectionMatrix());
    auto viewProjectionMatrix = projectionMatrix * viewMatrix;

    forwardDrawBinds["viewProjectionMatrix"] = viewProjectionMatrix;
    forwardDrawBinds["worldMatrix"] = worldModel->worldMatrix;
    forwardDrawBinds["Lights"] = &lightManager->GetLightsBuffer();
}

/*std::string LightCullNormal::GetForwardShaderPath()
{
    return "lightCullNormal/forward.frag";
}

std::string LightCullNormal::GetForwardShaderDebugPath()
{
    return "lightCullNormal/forwardDebug.frag";
}*/
