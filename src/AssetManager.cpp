/*
 * ---------------------------------------------------
 * AssetManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:00:28
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Math/Matrix.hpp"
#include "GPURessourceManager.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/RuntimeError.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"

#define POST_PROCESSING_FLAGS         \
    aiProcess_JoinIdenticalVertices | \
    aiProcess_MakeLeftHanded        | \
    aiProcess_Triangulate           | \
    aiProcess_GenSmoothNormals      | \
    aiProcess_FixInfacingNormals    | \
    aiProcess_FlipUVs               | \
    aiProcess_FlipWindingOrder      | \
    aiProcess_CalcTangentSpace

namespace GE
{

Mesh AssetManager::getMesh(const utils::String& filepath)
{
    using Iterator = utils::Dictionary<utils::String, Mesh>::Iterator;
    Iterator it = m_cachedMeshes.find(filepath);
    if (it == m_cachedMeshes.end())
        it = m_cachedMeshes.insert(filepath, loadMesh(filepath));
    return it->val;
}

Mesh AssetManager::loadMesh(const utils::String& filepath)
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filepath, POST_PROCESSING_FLAGS);
    if (scene == nullptr)
        throw utils::RuntimeError("fail to load the model using assimp");

    utils::Array<SubMesh> allMeshes;

    for(utils::uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        aiMesh* aiMesh = scene->mMeshes[meshIndex];

        SubMesh newSubMesh;

        newSubMesh.name = aiMesh->mName.C_Str();

        gfx::Buffer::Descriptor bufferDescriptor;
        bufferDescriptor.size = aiMesh->mNumVertices * sizeof(Renderer::Vertex);
        newSubMesh.vertexBuffer = GPURessourceManager::shared().newBuffer(bufferDescriptor);
        auto* vertices = (Renderer::Vertex*)newSubMesh.vertexBuffer->mapContent();

        bufferDescriptor.size = aiMesh->mNumFaces * 3UL * sizeof(utils::uint32);
        newSubMesh.indexBuffer = GPURessourceManager::shared().newBuffer(bufferDescriptor);
        auto* indices = (utils::uint32*)newSubMesh.indexBuffer->mapContent();

        bufferDescriptor.size = sizeof(math::mat4x4);
        newSubMesh.modelMatrixBuffer = GPURessourceManager::shared().newBuffer(bufferDescriptor);

        for(utils::uint32 i = 0; i < aiMesh->mNumVertices; i++)
        {
            vertices[i].pos = {
                aiMesh->mVertices[i].x,
                aiMesh->mVertices[i].y,
                aiMesh->mVertices[i].z
            };
            if (aiMesh->mTextureCoords[0] != nullptr)
            {
                vertices[i].uv = {
                    aiMesh->mTextureCoords[0][i].x,
                    aiMesh->mTextureCoords[0][i].y
                };
            }
            if (aiMesh->mNormals != nullptr)
            {
                vertices[i].normal = {
                    aiMesh->mNormals[i].x,
                    aiMesh->mNormals[i].y,
                    aiMesh->mNormals[i].z
                };
            }
            if (aiMesh->mTangents != nullptr)
            {
                vertices[i].tangent = {
                    aiMesh->mTangents[i].x,
                    aiMesh->mTangents[i].y,
                    aiMesh->mTangents[i].z
                };
            }
        }

        for(utils::uint32 i = 0; i < aiMesh->mNumFaces; i++)
        {
            indices[i * 3 + 0] = aiMesh->mFaces[i].mIndices[0];
            indices[i * 3 + 1] = aiMesh->mFaces[i].mIndices[1];
            indices[i * 3 + 2] = aiMesh->mFaces[i].mIndices[2];
        }

        newSubMesh.vertexBuffer->unMapContent();
        newSubMesh.indexBuffer->unMapContent();

        allMeshes.append(newSubMesh);
    }

    Mesh output;

    utils::Func<void(aiNode*, math::mat4x4)> addNode = [&](aiNode* aiNode, math::mat4x4 additionalTransform) {

        math::mat4x4 transform = additionalTransform * math::mat4x4(
            aiNode->mTransformation.a1, aiNode->mTransformation.a2, aiNode->mTransformation.a3, aiNode->mTransformation.a4,
            aiNode->mTransformation.b1, aiNode->mTransformation.b2, aiNode->mTransformation.b3, aiNode->mTransformation.b4,
            aiNode->mTransformation.c1, aiNode->mTransformation.c2, aiNode->mTransformation.c3, aiNode->mTransformation.c4,
            aiNode->mTransformation.d1, aiNode->mTransformation.d2, aiNode->mTransformation.d3, aiNode->mTransformation.d4
        );

        for (utils::uint32 i = 0; i < aiNode->mNumMeshes; i++)
        {
            SubMesh& subMesh = allMeshes[aiNode->mMeshes[i]];
            subMesh.transform = transform;
            output.subMeshes.append(subMesh);
        }

        for (utils::uint32 i = 0; i < aiNode->mNumChildren; i++)
            addNode(aiNode->mChildren[i], transform);
    };

    output.name = scene->mRootNode->mName.C_Str();

    for (utils::uint32 i = 0; i < scene->mRootNode->mNumMeshes; i++)
        output.subMeshes.append(allMeshes[scene->mRootNode->mMeshes[i]]);

    for (utils::uint32 i = 0; i < scene->mRootNode->mNumChildren; i++)
        addNode(scene->mRootNode->mChildren[i], math::mat4x4(1.0F));

    return output;
}

}