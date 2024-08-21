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

#include "Game-Engine/Material.hpp"
#include "Game-Engine/Mesh.hpp"
#include "Graphics/Texture.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class AssetManager
{
public:
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&)      = delete;

    static inline void init() { s_sharedInstance = utils::UniquePtr<AssetManager>(new AssetManager()); }
    static inline AssetManager& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    Mesh loadMesh(const utils::String& filepath);
    void unloadMesh(const utils::String& filepath);

    template<typename T>
    utils::SharedPtr<Material> newMaterial(const utils::String& name);
    inline utils::SharedPtr<Material> getMaterial(const utils::String& name) { return m_cachedMaterials[name]; }

    utils::SharedPtr<gfx::Texture> loadTexture(const utils::String& filePath);
    void unloadTexture(const utils::String& filePath);

    ~AssetManager() = default;

private:
    AssetManager() = default;

    inline static utils::UniquePtr<AssetManager> s_sharedInstance;

    utils::Dictionary<utils::String, Mesh> m_cachedMeshes;
    utils::Dictionary<utils::String, utils::SharedPtr<Material>> m_cachedMaterials;
    utils::Dictionary<utils::String, utils::SharedPtr<gfx::Texture>> m_cachedTextures;

public:
    AssetManager& operator = (const AssetManager&) = delete;
    AssetManager& operator = (AssetManager&&)      = delete;
};

template<typename T>
utils::SharedPtr<Material> AssetManager::newMaterial(const utils::String &name)
{
    utils::uint32 count = 1;
    utils::String searchedName = name;
    while (m_cachedMaterials.contain(searchedName))
    {
        searchedName = ++count == 2 ? name : name.substr(0, name.length() - 1);
        searchedName = searchedName + utils::String::fromUInt(count);
    }
    utils::SharedPtr<Material> newMaterial = utils::makeShared<T>().template staticCast<Material>();
    newMaterial->name = searchedName;
    return m_cachedMaterials.insert(searchedName, std::move(newMaterial))->val;
}

}

#endif // ASSETMANAGER_HPP