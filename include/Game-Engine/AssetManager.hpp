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

#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class Mesh;

class AssetManager
{
public:
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&)      = delete;

    static inline void init() { s_sharedInstance = utils::UniquePtr<AssetManager>(new AssetManager()); }
    static inline AssetManager& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    utils::SharedPtr<Mesh> getMesh(const utils::String& filepath);

    ~AssetManager() = default;

private:
    AssetManager() = default;

    utils::SharedPtr<Mesh> loadMesh(const utils::String& filepath);

    inline static utils::UniquePtr<AssetManager> s_sharedInstance;

    utils::Dictionary<utils::String, utils::SharedPtr<Mesh>> m_cachedMeshes;

public:
    AssetManager& operator = (const AssetManager&) = delete;
    AssetManager& operator = (AssetManager&&)      = delete;

};

}

#endif // ASSETMANAGER_HPP