#include "OBJModel.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "texture.h"

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
    //LibOBJ::OBJModel model;

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filePath
                                             , aiProcess_GenSmoothNormals
                                               | aiProcess_JoinIdenticalVertices
                                               | aiProcess_Triangulate
                                               | aiProcess_FlipWindingOrder); // Sponza is apparently flipped

    // TODO: Log?
    if(!scene)
        return CONTENT_ERROR_CODES::COULDNT_OPEN_CONTENT_FILE;

    if((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0)
        return CONTENT_ERROR_CODES::COULDNT_OPEN_DEPENDENCY_FILE;

    std::map<Texture*, std::pair<std::vector<LibOBJ::Vertex>, std::vector<GLint>>> vertexIndex;

    for(int i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[i];

        aiString textureName;
        aiGetMaterialString(scene->mMaterials[mesh->mMaterialIndex], AI_MATKEY_TEXTURE_DIFFUSE(0), &textureName);

        if(textureName.length == 0)
            continue; // TODO: Handle this case somehow

        Texture* currentTexture = contentManager->Load<Texture>(textureName.data);
        if(currentTexture == nullptr)
            return CONTENT_ERROR_CODES::COULDNT_OPEN_DEPENDENCY_FILE;

        std::vector<LibOBJ::Vertex>& vertices = vertexIndex[currentTexture].first;
        std::vector<GLint>& indicies = vertexIndex[currentTexture].second;

        auto indexOffset = vertices.size();
        vertices.reserve(vertices.size() + mesh->mNumVertices);

        for(int j = 0; j < mesh->mNumVertices; ++j)
        {
            LibOBJ::Vertex newVertex;

            newVertex.position.x = mesh->mVertices[j].x;
            newVertex.position.y = mesh->mVertices[j].y;
            newVertex.position.z = mesh->mVertices[j].z;

            newVertex.normal.x = mesh->mNormals[j].x;
            newVertex.normal.y = mesh->mNormals[j].y;
            newVertex.normal.z = mesh->mNormals[j].z;

            newVertex.texCoord.x = mesh->mTextureCoords[0][j].x;
            newVertex.texCoord.y = mesh->mTextureCoords[0][j].y;

            vertices.push_back(newVertex);
        }

        indicies.reserve(indicies.size() + mesh->mNumFaces);
        for(int j = 0; j < mesh->mNumFaces; ++j)
            for(int k = 0; k < mesh->mFaces[j].mNumIndices; ++k)
                indicies.push_back(indexOffset + mesh->mFaces[j].mIndices[k]);
    }

    std::vector<LibOBJ::Vertex> vertices;
    std::vector<GLint> indicies;

    for(const auto& drawData : vertexIndex)
    {
        const auto indexOffset = vertices.size();

        drawOffsets.insert(std::make_pair(drawData.first, std::make_pair(indicies.size(), drawData.second.second.size())));

        for(const auto index : drawData.second.second)
            indicies.push_back(index + indexOffset);

        vertices.insert(vertices.end()
                        , std::make_move_iterator(drawData.second.first.begin())
                        , std::make_move_iterator(drawData.second.first.end()));
    }

    vertexBuffer.Init<LibOBJ::Vertex, glm::vec3, glm::vec3, glm::vec2>(GLEnums::BUFFER_USAGE::STATIC, vertices);
    indexBuffer.Init(GLEnums::BUFFER_USAGE::STATIC, indicies);

    if(!drawBinds.AddShaders(*contentManager
                             , GLEnums::SHADER_TYPE::VERTEX, "forward.vert"
                             , GLEnums::SHADER_TYPE::PIXEL, "forward.frag"))
        return CONTENT_ERROR_CODES::COULDNT_OPEN_CONTENT_FILE;

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

    for(const auto& pair : drawOffsets)
    {
        glBindTexture(GL_TEXTURE_2D, pair.first->GetTexture());
        drawBinds.DrawElements(pair.second.second, pair.second.first);
    }

    //int i = 0;
//
    //for(const auto& pair : materialOffset)
    //{
    //    if(!pair.first.textureName.empty())
    //        glBindTexture(GL_TEXTURE_2D, textureNameToTexture[pair.first.textureName]->GetTexture());
    //    else
    //        glBindTexture(GL_TEXTURE_2D, NULL);
//
    //    if((drawOnlyIndex != -1 && i == drawOnlyIndex) || drawOnlyIndex == -1)
    //        drawBinds.DrawElements(pair.second.second, pair.second.first);
//
    //    ++i;
    //}

    drawBinds.Unbind();
}
