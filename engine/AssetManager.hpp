/*
 * ---------------------------------------------------
 * AssetManager.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/13 11:49:48
 * ---------------------------------------------------
 */

#ifndef ASSETMANAGER_HPP
#define ASSETMANAGER_HPP

#include "Renderer/Mesh.hpp"
#include "Graphics/GraphicAPI.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"
#include "crossguid/guid.hpp"

namespace GE
{

using AssetID = xg::Guid;

class AssetManager
{
public:
    AssetManager()                    = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&)      = default;

    utils::String assetShortPath(AssetID, const utils::String& ressourceDirFullPath);

    AssetID registerMesh(const utils::String& fullPath);
    inline const utils::Set<AssetID>& registeredMeshes() const { return m_registeredMeshes; }

    inline Mesh& loadedMesh(AssetID id) { return m_loadedMeshes[id]; }

    void loadAssets(gfx::GraphicAPI&);
    void unloadAssets();
    inline bool isLoaded() const { return m_api != nullptr; }

    ~AssetManager() = default;

private:
    Mesh loadMesh(const utils::String& filepath, gfx::GraphicAPI&);

    gfx::GraphicAPI* m_api = nullptr;

    utils::Dictionary<AssetID, utils::String> m_assetFullPaths;

    utils::Set<AssetID> m_registeredMeshes;

    utils::Dictionary<AssetID, Mesh> m_loadedMeshes;

public:
    AssetManager& operator = (const AssetManager&) = delete;
    AssetManager& operator = (AssetManager&&)      = default;

};

}

#endif // ASSETMANAGER_HPP