#include <glm/gtc/matrix_transform.hpp>
#include <set>
#include "OBJModel.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "texture.h"
#include "textureCreationParameters.h"

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


    for(int i = 0; i < scene->mNumMaterials; ++i)
    {
        Material newMaterial;

        aiMaterial* material = scene->mMaterials[i];

        aiString materialName;
        aiGetMaterialString(material, AI_MATKEY_NAME, &materialName);

        unsigned int channels = 3;
        aiGetMaterialFloatArray(material, AI_MATKEY_COLOR_AMBIENT, &newMaterial.ambientColor.x, &channels);
        channels = 3;
        aiGetMaterialFloatArray(material, AI_MATKEY_COLOR_DIFFUSE, &newMaterial.diffuseColor.x, &channels);
        aiGetMaterialFloat(material, AI_MATKEY_OPACITY, &newMaterial.opacity);
        newMaterial.specularExponent = 1.0f;

        aiString textureName;
        aiGetMaterialString(material, AI_MATKEY_TEXTURE_DIFFUSE(0), &textureName);

        Texture* newTexture;

        if(textureName.length == 0)
        {
            if(!contentManager->HasCreated("whiteTexture"))
            {
                Logger::LogLine(LOG_TYPE::FATAL, "whiteTexture needs to be created before OBJ is loaded if it is needed");
                return CONTENT_ERROR_CODES::COULDNT_OPEN_DEPENDENCY_FILE;
            }

            TextureCreationParameters parameters("whiteTexture");

            newTexture = contentManager->Load<Texture>("", &parameters);
        }
        else
        {
            newTexture = contentManager->Load<Texture>(textureName.data);
            if(newTexture == nullptr)
                return CONTENT_ERROR_CODES::COULDNT_OPEN_DEPENDENCY_FILE;
        }

        newMaterial.texture = newTexture;

        materials.push_back(newMaterial);
    }

    std::vector<LibOBJ::Vertex> vertices;
    std::vector<GLint> indices;

    std::vector<aiMesh*> opaqueMeshes;
    std::vector<aiMesh*> transparentMeshes;

    for(int i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[i];

        if(materials[mesh->mMaterialIndex].opacity < 1.0f)
            transparentMeshes.push_back(mesh);
        else
            opaqueMeshes.push_back(mesh);
    }
    
    for(aiMesh* mesh : opaqueMeshes)
    {
        DrawData newDrawData;

        auto indexOffset = vertices.size();
        vertices.reserve(mesh->mNumVertices);

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

        newDrawData.indexOffset = (int)indices.size();

        indices.reserve(indices.size() + mesh->mNumFaces * 3);
        for(int j = 0; j < mesh->mNumFaces; ++j)
            for(int k = 0; k < mesh->mFaces[j].mNumIndices; ++k)
                indices.push_back(indexOffset + mesh->mFaces[j].mIndices[k]);

        newDrawData.indexCount = (int)(indices.size() - newDrawData.indexOffset);
        newDrawData.materialIndex = mesh->mMaterialIndex;
        
        opaqueDrawData.push_back(newDrawData);
    }

    const static float SCALE = 0.01f;
    worldMatrix = glm::scale(glm::mat4(), glm::vec3(SCALE)); // TODO

    for(aiMesh* mesh : transparentMeshes)
    {
        DrawData newDrawData;

        auto indexOffset = vertices.size();
        vertices.reserve(mesh->mNumVertices);

        glm::vec3 minPos = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 maxPos = glm::vec3(std::numeric_limits<float>::min());

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

            minPos = glm::min(minPos, newVertex.position * SCALE);
            maxPos = glm::max(maxPos, newVertex.position * SCALE);

            vertices.push_back(newVertex);
        }

        newDrawData.centerPosition = minPos + (maxPos - minPos) * 0.5f;
        newDrawData.indexOffset = (int)indices.size();

        indices.reserve(indices.size() + mesh->mNumFaces * 3);
        for(int j = 0; j < mesh->mNumFaces; ++j)
            for(int k = 0; k < mesh->mFaces[j].mNumIndices; ++k)
                indices.push_back(indexOffset + mesh->mFaces[j].mIndices[k]);

        newDrawData.indexCount = (int)(indices.size() - newDrawData.indexOffset);
        newDrawData.materialIndex = mesh->mMaterialIndex;

        transparentDrawData.push_back(newDrawData);
    }

    vertexBuffer.Init<LibOBJ::Vertex, glm::vec3, glm::vec3, glm::vec2>(GLEnums::BUFFER_USAGE::STATIC, vertices);
    indexBuffer.Init(GLEnums::BUFFER_USAGE::STATIC, indices);

    if(!drawBinds.AddShaders(*contentManager
                             , GLEnums::SHADER_TYPE::VERTEX, "forward.vert"
                             , GLEnums::SHADER_TYPE::PIXEL, "forward.frag"))
        return CONTENT_ERROR_CODES::COULDNT_OPEN_CONTENT_FILE;

    GLInputLayout vertexBufferLayout;
    vertexBufferLayout.SetInputLayout<glm::vec3, glm::vec3, glm::vec2>();

    drawBinds.AddBuffers(&indexBuffer
                         , &vertexBuffer, vertexBufferLayout);

    drawBinds.AddUniform("viewProjectionMatrix", glm::mat4x4());
    drawBinds.AddUniform("worldMatrix", worldMatrix);
    drawBinds.AddUniform("materialIndex", 0);

    if(!drawBinds.Init())
        return CONTENT_ERROR_CODES::CREATE_FROM_MEMORY;

    std::vector<GPUMaterial> gpuMaterials;
    gpuMaterials.reserve(materials.size());

    for(const auto& material : materials)
        gpuMaterials.push_back(GPUMaterial(material));

    GLUniformBuffer* materialsBuffer = drawBinds.GetUniformBuffer("Materials");
    materialsBuffer->SetData(&gpuMaterials[0], sizeof(GPUMaterial) * gpuMaterials.size());

    struct LightData
    {
        glm::vec3 lightPosition;
        float padding0;
        glm::vec3 lightColor;
        float padding1;
    } lightData;

    lightData.lightPosition = glm::vec3(0.0f, 5.0f, 0.0f);
    lightData.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    GLUniformBuffer* lightBuffer = drawBinds.GetUniformBuffer("LightData");
    lightBuffer->SetData(&lightData, sizeof(LightData));

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

void OBJModel::Draw(const glm::vec3 cameraPosition)
{
    drawBinds.Bind();

    for(const auto& data : opaqueDrawData)
    {
        *drawBinds["materialIndex"] = data.materialIndex;
        drawBinds["materialIndex"]->UploadData();

        glBindTexture(GL_TEXTURE_2D, materials[data.materialIndex].texture->GetTexture());
        drawBinds.DrawElements(data.indexCount, data.indexOffset);
    }

    transparentDrawData[0].distanceToCamera = glm::distance(cameraPosition, transparentDrawData[0].centerPosition);

    for(int i = 1; i < transparentDrawData.size(); ++i)
    {
        transparentDrawData[i].distanceToCamera = glm::distance(cameraPosition, transparentDrawData[i].centerPosition);

        for(int j = i; j > 0; --j)
        {
            if(transparentDrawData[j].distanceToCamera > transparentDrawData[j - 1].distanceToCamera)
                std::swap(transparentDrawData[j], transparentDrawData[j - 1]);
            else
                break;
        }
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    for(const auto& data : transparentDrawData)
    {
        *drawBinds["materialIndex"] = data.materialIndex;
        drawBinds["materialIndex"]->UploadData();

        glBindTexture(GL_TEXTURE_2D, materials[data.materialIndex].texture->GetTexture());
        drawBinds.DrawElements(data.indexCount, data.indexOffset);
    }

    glDisable(GL_BLEND);

    drawBinds.Unbind();
}
