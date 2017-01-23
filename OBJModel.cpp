#include "OBJModel.h"

#include "libobj.h"

OBJModel::OBJModel()
{}

OBJModel::~OBJModel()
{}

int OBJModel::GetStaticVRAMUsage() const
{
    return 0;
}

int OBJModel::GetDynamicVRAMUsage() const
{
    return 0;
}

int OBJModel::GetRAMUsage() const
{
    return 0;
}

bool OBJModel::CreateDefaultContent(const char* filePath, ContentManager* contentManager)
{
    return false;
}

CONTENT_ERROR_CODES OBJModel::Load(const char* filePath
                                   , ContentManager* contentManager
                                   , ContentParameters* contentParameters)
{
    LibOBJ::OBJModel model;

    try
    {
        model = LibOBJ::ReadModel(filePath);
    }
    catch(std::runtime_error& ex)
    {
        Logger::LogLine(LOG_TYPE::FATAL, ex.what());

        return CONTENT_ERROR_CODES::COULDNT_OPEN_FILE;
    }

    int totalSize = 0;
    for(const auto& pair : model.vertices)
        totalSize += pair.second.size();

    std::unique_ptr<LibOBJ::Vertex> vertices((LibOBJ::Vertex*)malloc(sizeof(LibOBJ::Vertex) * totalSize));

    int offset = 0;
    for(const auto& pair : model.vertices)
    {
        std::memcpy(vertices.get() + offset, &pair.second[0], sizeof(LibOBJ::Vertex) * pair.second.size());
        materialOffset.push_back(std::make_pair(model.materials[pair.first], std::make_pair(offset, pair.second.size())));

        offset += pair.second.size();
    }

    vertexBuffer.Init<glm::vec3, glm::vec3, glm::vec2>(GLEnums::BUFFER_USAGE::STATIC, vertices.get(), totalSize);

    std::vector<GLint> indicies; // TODO: Generate this when loading
    for(int i = 0; i < totalSize; ++i)
        indicies.push_back(indicies.size());

    indexBuffer.Init(GLEnums::BUFFER_USAGE::STATIC, indicies);

    drawBinds.AddShaders(*contentManager
                     , GLEnums::SHADER_TYPE::VERTEX, "vertex.glsl"
                     , GLEnums::SHADER_TYPE::PIXEL, "pixel.glsl");

    GLInputLayout vertexBufferLayout;
    vertexBufferLayout.SetInputLayout<glm::vec3, glm::vec3, glm::vec2>();

    drawBinds.AddBuffers(&indexBuffer
                         , &vertexBuffer, vertexBufferLayout);

    drawBinds.AddUniform("viewProjectionMatrix", glm::mat4x4());

    if(!drawBinds.Init())
        return CONTENT_ERROR_CODES::CREATE_FROM_MEMORY;

    return CONTENT_ERROR_CODES::NONE;
}

void OBJModel::Unload(ContentManager* contentManager)
{

}

CONTENT_ERROR_CODES OBJModel::BeginHotReload(const char* filePath, ContentManager* contentManager)
{
    return CONTENT_ERROR_CODES::NONE;
}

bool OBJModel::ApplyHotReload()
{
    return false;
}

bool OBJModel::Apply(Content* content)
{
    return false;
}

DiskContent* OBJModel::CreateInstance() const
{
    return new OBJModel;
}

void OBJModel::Draw()
{
    drawBinds.Bind();

    int i = 0;

    for(const auto& pair : materialOffset)
    {
        if((drawOnlyIndex != -1 && i == drawOnlyIndex) || drawOnlyIndex == -1)
            drawBinds.DrawElements(pair.second.first, pair.second.second);

        ++i;
    }

    drawBinds.Unbind();
}
