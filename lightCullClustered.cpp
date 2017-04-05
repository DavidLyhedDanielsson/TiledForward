#include "lightCullClustered.h"

LightCullClustered::LightCullClustered()
{}

LightCullClustered::~LightCullClustered()
{}

bool LightCullClustered::Init(ContentManager& contentManager, Console& console)
{
    if(!Base::Init(contentManager, console))
        return false;

    ShaderContentParameters parameters;
    parameters.type = GLEnums::SHADER_TYPE::COMPUTE;
    parameters.variables.push_back(std::make_pair("TREE_MAX_DEPTH", std::to_string(GetTreeMaxDepth())));
    lightClusterDrawBinds.AddShader(contentManager, parameters,  "lightCullClustered/lightCluster.comp");
    if(!lightClusterDrawBinds.Init())
        return false;

    lightClusterDrawBinds["Lights"] = lightCullDrawBinds["Lights"];
    lightClusterDrawBinds["Tree"] = lightCullDrawBinds["Tree"];
    lightClusterDrawBinds["LightIndices"] = lightCullDrawBinds["LightIndices"];
    lightClusterDrawBinds["TileLights"] = lightCullDrawBinds["TileLights"];
    lightClusterDrawBinds["ReadWriteOffsets"] = lightCullDrawBinds["ReadWriteOffsets"];

    int tileCountX = (int)std::pow(2, GetTreeMaxDepth());
    int tileCountY = (int)std::pow(2, GetTreeMaxDepth());

    lightClusterDrawBinds.GetSSBO("FinalLights")->SetData(nullptr, tileCountX * tileCountY * sizeof(float) * 8);

    return true;
}

std::string LightCullClustered::GetForwardShaderPath()
{
    return "lightCullClustered/forward.frag";
    return Base::GetForwardShaderPath();
}

std::string LightCullClustered::GetForwardShaderDebugPath()
{
    return "lightCullClustered/forwardDebug.frag";
    return Base::GetForwardShaderDebugPath();
}

void LightCullClustered::Draw()
{
    LightCullAdaptive::Draw();

    lightReductionDrawBinds.Unbind();

    lightClusterDrawBinds.Bind();

    GLuint tileCountX = (GLuint)std::pow(2, GetTreeMaxDepth());
    GLuint tileCountY = (GLuint)std::pow(2, GetTreeMaxDepth());

    glDispatchCompute(tileCountX, tileCountY, 1);

    lightClusterDrawBinds.Unbind();
}

void LightCullClustered::SetDrawBindData(GLDrawBinds& binds)
{
    binds["ScreenSize"] = lightCullDrawBinds["ScreenSize"];
    binds["FinalLights"] = lightClusterDrawBinds["FinalLights"];
    binds["Lights"] = lightClusterDrawBinds["Lights"];

    //tileCountXLocation = glGetUniformLocation(binds.GetShader(GLEnums::SHADER_TYPE::FRAGMENT, 0)->GetShader(), "tileCountX");
    //tileCountX = (int)std::pow(2, TREE_MAX_DEPTH);
    //binds["tileCountX"] = tileCountX;

    //Base::SetDrawBindData(binds);
    //return;
}
