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
    lightSortDrawBinds.AddUniform("startIndex", 0);

    ShaderContentParameters parameters;
    parameters.type = GLEnums::SHADER_TYPE::COMPUTE;
    parameters.variables.push_back(std::make_pair("TREE_MAX_DEPTH", std::to_string(GetTreeMaxDepth())));
    parameters.variables.push_back(std::make_pair("MAX_LIGHTS_PER_TILE", std::to_string(GetMaxLightsPerTile())));
    lightSortDrawBinds.AddShader(contentManager, parameters, "lightCullClustered/lightSort.comp");
    if(!lightSortDrawBinds.Init())
        return false;

    lightSortDrawBinds["Lights"] = lightCullDrawBinds["Lights"];
    lightSortDrawBinds["LightIndices"] = lightCullDrawBinds["LightIndices"];
    lightSortDrawBinds["ReadWriteOffsets"] = lightCullDrawBinds["ReadWriteOffsets"];
    lightSortDrawBinds["logSize"] = (int)log2(MAX_LIGHTS_PER_TILE);


    lightSortDrawBinds["OutLightIndices"] = std::vector<int>((unsigned long)(GetMaxNumberOfTiles()
                                                                             * GetMaxLightsPerTile())
                                                             , 0);

    int tileCountX = (int)std::pow(2, GetTreeMaxDepth());
    int tileCountY = (int)std::pow(2, GetTreeMaxDepth());

    //lightClusterDrawBinds.GetSSBO("FinalLights")->SetData(nullptr, tileCountX * tileCountY * sizeof(float) * 8);

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

void LightCullClustered::Draw()
{
    Base::Draw();

    if(sort)
    {
        lightReductionDrawBinds.Unbind();

        lightSortDrawBinds.Bind();

        GLuint tileCountX = (GLuint)std::pow(2, GetTreeMaxDepth());
        GLuint tileCountY = (GLuint)std::pow(2, GetTreeMaxDepth());

        for(int y = 0; y < tileCountY; ++y)
        {
            for(int x = 0; x < tileCountX; ++x)
            {
                lightSortDrawBinds["startIndex"] = (int)((y * tileCountX + x) * MAX_LIGHTS_PER_TILE);

                glDispatchCompute(1, 1, 1);
            }
        }

        //glDispatchCompute(tileCountX, tileCountY, 1);

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