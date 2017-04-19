#include "lightCullClustered.h"
#include "console/commandGetSet.h"

LightCullClustered::LightCullClustered()
    : sort(true)
{}

LightCullClustered::~LightCullClustered()
{}

bool LightCullClustered::Init(ContentManager& contentManager, Console& console)
{
    if(!Base::Init(contentManager, console))
        return false;

    console.AddCommand(new CommandGetSet<bool>("light_sort", &sort));

    lightSortDrawBinds.AddUniform("logSize", 0);

    ShaderContentParameters sortParameters;
    sortParameters.type = GLEnums::SHADER_TYPE::COMPUTE;
    sortParameters.variables.push_back(std::make_pair("TREE_MAX_DEPTH", std::to_string(GetTreeMaxDepth())));
    sortParameters.variables.push_back(std::make_pair("MAX_LIGHTS_PER_TILE", std::to_string(GetMaxLightsPerTile())));
    sortParameters.variables.push_back(std::make_pair("TILE_COUNT_X", std::to_string((GLuint)std::pow(2, GetTreeMaxDepth()))));
    lightSortDrawBinds.AddShader(contentManager, sortParameters, "lightCullClustered/lightSort.comp");
    if(!lightSortDrawBinds.Init())
        return false;

    lightSortDrawBinds["Lights"] = lightCullDrawBinds["Lights"];
    lightSortDrawBinds["LightIndices"] = lightCullDrawBinds["LightIndices"];
    lightSortDrawBinds["ReadWriteOffsets"] = lightCullDrawBinds["ReadWriteOffsets"];
    lightSortDrawBinds["logSize"] = (int)log2(MAX_LIGHTS_PER_TILE);


    lightSortDrawBinds["OutLightIndices"] = std::vector<int>((unsigned long)(GetMaxNumberOfTiles()
                                                                             * GetMaxLightsPerTile())
                                                             , 0);

    ShaderContentParameters clusterParameters;
    clusterParameters.type = GLEnums::SHADER_TYPE::COMPUTE;
    clusterParameters.variables.push_back(std::make_pair("TREE_MAX_DEPTH", std::to_string(GetTreeMaxDepth())));
    clusterParameters.variables.push_back(std::make_pair("BUCKET_COUNT", std::to_string(BUCKET_COUNT)));
    clusterParameters.variables.push_back(std::make_pair("MAX_LIGHTS_PER_TILE", std::to_string(GetMaxLightsPerTile())));
    lightClusterDrawBinds.AddShader(contentManager, clusterParameters, "lightCullClustered/lightCluster.comp");
    if(!lightClusterDrawBinds.Init())
        return false;

    lightClusterDrawBinds["Lights"] = lightCullDrawBinds["Lights"];
    lightClusterDrawBinds["LightIndices"] = lightCullDrawBinds["LightIndices"];
    lightClusterDrawBinds["ReadWriteOffsets"] = lightCullDrawBinds["ReadWriteOffsets"];
    lightClusterDrawBinds["FinalLights"] = std::vector<float>((unsigned long)(GetMaxNumberOfTiles() * 8 * BUCKET_COUNT), 0);
    lightClusterDrawBinds["TileLights"] = lightCullDrawBinds["TileLights"];
    lightClusterDrawBinds["Tree"] = lightCullDrawBinds["Tree"];

    return true;
}

/*std::string LightCullClustered::GetForwardShaderPath()
{
    //return "lightCullClustered/forward.frag";
    return Base::GetForwardShaderPath();
}

std::string LightCullClustered::GetForwardShaderDebugPath()
{
    //return "lightCullClustered/forwardDebug.frag";
    return Base::GetForwardShaderDebugPath();
}*/

void LightCullClustered::Draw(std::vector<std::pair<std::string, GLuint64>>& times)
{
    Base::Draw(times);

    if(sort)
    {
        StartTimeQuery();

        lightReductionDrawBinds.Unbind();

        GLuint tileCountX = (GLuint)std::pow(2, GetTreeMaxDepth());
        GLuint tileCountY = (GLuint)std::pow(2, GetTreeMaxDepth());

        lightSortDrawBinds.Bind();
        glDispatchCompute(tileCountX, tileCountY, 1);
        lightSortDrawBinds.Unbind();

        times.push_back(std::make_pair("Light sort", StopTimeQuery()));

        StartTimeQuery();

        lightClusterDrawBinds.Bind();
        glDispatchCompute(tileCountX, tileCountY, 1);
        lightClusterDrawBinds.Unbind();

        times.push_back(std::make_pair("Light cluster", StopTimeQuery()));
    }
}

void LightCullClustered::PostDraw()
{
    if(sort)
        lightSortDrawBinds.Unbind();
    else
        lightReductionDrawBinds.Unbind();
}

void LightCullClustered::SetDrawBindData()
{
    //binds["ScreenSize"] = lightCullDrawBinds["ScreenSize"];
    //binds["FinalLights"] = lightClusterDrawBinds["FinalLights"];
    //binds["Lights"] = lightClusterDrawBinds["Lights"];

    //tileCountXLocation = glGetUniformLocation(binds.GetShader(GLEnums::SHADER_TYPE::FRAGMENT, 0)->GetShader(), "tileCountX");
    //tileCountX = (int)std::pow(2, TREE_MAX_DEPTH);
    //binds["tileCountX"] = tileCountX;

    Base::SetDrawBindData();
    //return;
}

int LightCullClustered::GetTileCountX() const
{
    return (int)std::pow(2, GetTreeMaxDepth());
}

std::vector<std::pair<std::string, GLuint64>> LightCullClustered::TimedDraw(glm::mat4 viewMatrix
                                       , glm::mat4 projectionMatrixInverse
                                       , LightManager& lightManager)
{
    std::vector<std::pair<std::string, GLuint64>> returnStuff;

    PreDraw(viewMatrix, projectionMatrixInverse, lightManager);

    Draw(returnStuff);

    PostDraw();

    return returnStuff;
}

void LightCullClustered::PreDraw(glm::mat4 viewMatrix, glm::mat4 projectionMatrixInverse, LightManager& lightManager)
{
    LightCullAdaptive::PreDraw(viewMatrix, projectionMatrixInverse, lightManager);
}
