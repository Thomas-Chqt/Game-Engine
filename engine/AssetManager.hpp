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
// #include "Graphics/Texture.hpp"
#include "UtilsCPP/Dictionary.hpp"
// #include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "crossguid/guid.hpp"

namespace GE
{

class AssetManager
{
public:
    AssetManager()                    = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&)      = default;

    xg::Guid registerMesh(const utils::String& name, const utils::String& filepath);
    utils::Dictionary<xg::Guid, utils::String> registeredMesh() const;

    utils::String getMeshName(xg::Guid id) { return id.isValid() ? m_registeredMeshes[id].name : "no mesh"; }
    Mesh& getLoadedMesh(xg::Guid id) { return m_loadedMeshes[id]; }

    void loadAssets(gfx::GraphicAPI&);
    void unloadAssets();
    inline bool isLoaded() const { return m_isLoaded; }

    ~AssetManager() = default;

private:
    struct RegisteredAsset
    {
        utils::String name;
        utils::String path;
    };

    Mesh loadMesh(const utils::String& filepath, gfx::GraphicAPI&);

    bool m_isLoaded = false;
    gfx::GraphicAPI* m_api = nullptr;

    utils::Dictionary<xg::Guid, RegisteredAsset> m_registeredMeshes;

    utils::Dictionary<xg::Guid, Mesh> m_loadedMeshes;

public:
    AssetManager& operator = (const AssetManager&) = delete;
    AssetManager& operator = (AssetManager&&)      = default;

};

}

#endif // ASSETMANAGER_HPP