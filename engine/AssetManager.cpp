/*
 * ---------------------------------------------------
 * AssetManager.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 12:00:28
 * ---------------------------------------------------
 */

#include "AssetManager.hpp"
#include "Graphics/Buffer.hpp"
#include "Renderer/Mesh.hpp"
#include "Math/Matrix.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/RuntimeError.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/Types.hpp"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/types.h"
#include "uuid.h"
#include <algorithm>
#include <array>
#include <cassert>
#include <filesystem>
#include <iterator>
#include <random>
#include <string>

using json = nlohmann::json;
namespace fs = std::filesystem;

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

AssetManager::AssetManager()
{
}

AssetID AssetManager::registerMesh(const fs::path& path)
{
    AssetID newAssetID;
    if (m_registeredMeshes.contain(path))
        newAssetID = m_registeredMeshes[path];
    else
    {
        std::random_device rd;
        auto seed_data = std::array<int, std::mt19937::state_size> {};
        std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
        std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
        std::mt19937 generator(seq);
        newAssetID = uuids::uuid_random_generator{generator}();
        m_registeredMeshes.insert(path, newAssetID);
    }

    if (isLoaded())
        m_loadedMeshes.insert(newAssetID, loadMesh(m_baseDir/path, *m_api));

    return newAssetID;
}

void AssetManager::loadAssets(gfx::GraphicAPI& api, const fs::path& baseDir)
{
    assert(baseDir.empty() || baseDir.is_absolute() && fs::is_directory(baseDir));

    m_api = &api;
    m_baseDir = baseDir;

    for (auto& [path, id] : m_registeredMeshes)
        m_loadedMeshes.insert(id, loadMesh(m_baseDir/path, *m_api));
    m_loadedMeshes.insert(BUILT_IN_CUBE_ASSET_ID, loadBuiltInCube());
}

void AssetManager::unloadAssets()
{
    m_loadedMeshes.clear();

    m_api = nullptr;
    m_baseDir.clear();
}

Mesh AssetManager::loadMesh(const fs::path& filepath, gfx::GraphicAPI& api)
{
    assert(fs::is_regular_file(filepath));

    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filepath.string(), POST_PROCESSING_FLAGS);
    if (scene == nullptr)
        throw utils::RuntimeError("fail to load the model using assimp");

    utils::Array<SubMesh> flatSubMeshes;

    for(utils::uint32 meshIndex = 0; meshIndex < scene->mNumMeshes; meshIndex++)
    {
        aiMesh* aiMesh = scene->mMeshes[meshIndex];

        gfx::Buffer::Descriptor bufferDescriptor;
        bufferDescriptor.size = aiMesh->mNumVertices * sizeof(Renderer::Vertex);
        utils::SharedPtr<gfx::Buffer> vertexBuffer = api.newBuffer(bufferDescriptor);
        auto* vertices = (Renderer::Vertex*)vertexBuffer->mapContent();

        bufferDescriptor.size = aiMesh->mNumFaces * 3UL * sizeof(utils::uint32);
        utils::SharedPtr<gfx::Buffer> indexBuffer = api.newBuffer(bufferDescriptor);
        auto* indices = (utils::uint32*)indexBuffer->mapContent();

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

        vertexBuffer->unMapContent();
        indexBuffer->unMapContent();

        flatSubMeshes.append(SubMesh{
            aiMesh->mName.C_Str(),
            math::mat4x4(1.0F),
            vertexBuffer,
            indexBuffer
        });
    }

    utils::Array<SubMesh> transformedSubMeshes;

    utils::Func<void(aiNode*, math::mat4x4)> addNode = [&](aiNode* aiNode, math::mat4x4 additionalTransform) {

        math::mat4x4 transform = additionalTransform * math::mat4x4(
            aiNode->mTransformation.a1, aiNode->mTransformation.a2, aiNode->mTransformation.a3, aiNode->mTransformation.a4,
            aiNode->mTransformation.b1, aiNode->mTransformation.b2, aiNode->mTransformation.b3, aiNode->mTransformation.b4,
            aiNode->mTransformation.c1, aiNode->mTransformation.c2, aiNode->mTransformation.c3, aiNode->mTransformation.c4,
            aiNode->mTransformation.d1, aiNode->mTransformation.d2, aiNode->mTransformation.d3, aiNode->mTransformation.d4
        );

        for (utils::uint32 i = 0; i < aiNode->mNumMeshes; i++)
        {
            SubMesh& flatSubMeshe = flatSubMeshes[aiNode->mMeshes[i]];
            transformedSubMeshes.append(SubMesh{
                flatSubMeshe.name,
                transform,
                flatSubMeshe.vertexBuffer,
                flatSubMeshe.indexBuffer
            });
        }

        for (utils::uint32 i = 0; i < aiNode->mNumChildren; i++)
            addNode(aiNode->mChildren[i], transform);
    };

    for (utils::uint32 i = 0; i < scene->mRootNode->mNumMeshes; i++)
        transformedSubMeshes.append(flatSubMeshes[scene->mRootNode->mMeshes[i]]);

    for (utils::uint32 i = 0; i < scene->mRootNode->mNumChildren; i++)
        addNode(scene->mRootNode->mChildren[i], math::mat4x4(1.0F));

    return Mesh{
        scene->mRootNode->mName.C_Str(),
        transformedSubMeshes
    };
}

Mesh AssetManager::loadBuiltInCube()
{
    Renderer::Vertex vertices[] = {
        { {-1, -1, -1}, {0, 1}, {-1,  0,  0}, { 0,  1,  0} },
        { {-1,  1, -1}, {1, 1}, {-1,  0,  0}, { 0,  1,  0} },
        { {-1,  1,  1}, {1, 0}, {-1,  0,  0}, { 0,  1,  0} },
        { {-1, -1,  1}, {0, 0}, {-1,  0,  0}, { 0,  1,  0} },
        { {-1, -1,  1}, {0, 1}, { 0,  0,  1}, { 0,  1,  1} },
        { {-1,  1,  1}, {1, 1}, { 0,  0,  1}, { 0,  1,  1} },
        { { 1,  1,  1}, {1, 0}, { 0,  0,  1}, { 0,  1,  1} },
        { { 1, -1,  1}, {0, 0}, { 0,  0,  1}, { 0,  1,  1} },
        { { 1, -1,  1}, {0, 1}, { 1,  0,  0}, { 0,  1,  0} },
        { { 1,  1,  1}, {1, 1}, { 1,  0,  0}, { 0,  1,  0} },
        { { 1,  1, -1}, {1, 0}, { 1,  0,  0}, { 0,  1,  0} },
        { { 1, -1, -1}, {0, 0}, { 1,  0,  0}, { 0,  1,  0} },
        { { 1, -1, -1}, {1, 0}, { 0,  0, -1}, { 0, -1, -1} },
        { { 1,  1, -1}, {0, 0}, { 0,  0, -1}, { 0, -1, -1} },
        { {-1,  1, -1}, {0, 1}, { 0,  0, -1}, { 0, -1, -1} },
        { {-1, -1, -1}, {1, 1}, { 0,  0, -1}, { 0, -1, -1} },
        { {-1, -1,  1}, {0, 1}, { 0, -1,  0}, { 1,  0,  0} },
        { { 1, -1,  1}, {1, 1}, { 0, -1,  0}, { 1,  0,  0} },
        { { 1, -1, -1}, {1, 0}, { 0, -1,  0}, { 1,  0,  0} },
        { {-1, -1, -1}, {0, 0}, { 0, -1,  0}, { 1,  0,  0} },
        { { 1,  1,  1}, {0, 1}, { 0,  1,  0}, {-1,  0,  0} },
        { {-1,  1,  1}, {1, 1}, { 0,  1,  0}, {-1,  0,  0} },
        { {-1,  1, -1}, {1, 0}, { 0,  1,  0}, {-1,  0,  0} },
        { { 1,  1, -1}, {0, 0}, { 0,  1,  0}, {-1,  0,  0} },    
    };

    utils::uint32 indices[] = {
         2,  1,  0,  3,  2,  0,
         6,  5,  4,  7,  6,  4,
        10,  9,  8, 11, 10,  8,
        14, 13, 12, 15, 14, 12,
        18, 17, 16, 19, 18, 16,
        22, 21, 20, 23, 22, 20
    };

    return Mesh {
        "built_in_cube",
        utils::Array<SubMesh> {{
            "built_in_cube_submesh",
            math::mat4x4(1.0F),
            m_api->newBuffer(gfx::Buffer::Descriptor(sizeof(Renderer::Vertex) * 24, vertices)),
            m_api->newBuffer(gfx::Buffer::Descriptor(sizeof(utils::uint32) * 36, indices)),
        }}
    };
}

void to_json(json& jsn, const AssetManager& assetManager)
{
    if (assetManager.m_registeredMeshes.isEmpty() == false)
    {
        json registeredMeshesJsn;
        for (auto& [path, id] : assetManager.m_registeredMeshes)
            registeredMeshesJsn[path.string()] = uuids::to_string(id);
        jsn["registeredMeshes"] = registeredMeshesJsn;
    }
}

void from_json(const json& jsn, AssetManager& assetManager)
{
    auto registeredMeshesIt = jsn.find("registeredMeshes");
    if (registeredMeshesIt != jsn.end())
    {
        for (auto& el : registeredMeshesIt->items())
        {
            auto id = uuids::uuid::from_string(el.value().get<std::string>());
            if (id.has_value())
                assetManager.m_registeredMeshes.insert(fs::path(el.key()), id.value());
        }
    }   
}

}