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
#include "uuid.h"
#include <filesystem>
#include <nlohmann/json.hpp>

namespace GE
{

using AssetID = uuids::uuid;

class AssetManager
{
public:
    AssetManager()                    = default;
    AssetManager(const AssetManager&) = default;
    AssetManager(AssetManager&&)      = default;

    AssetID registerMesh(const std::filesystem::path& relativePath);
    std::filesystem::path registeredMeshPath(AssetID id) const;
    inline const utils::Dictionary<std::filesystem::path, AssetID>& registeredMeshes() const { return m_registeredMeshes; }

    inline const Mesh& loadedMesh(AssetID id) const { return m_loadedMeshes[id]; }

    void loadAssets(gfx::GraphicAPI&, const std::filesystem::path& baseDir);
    void unloadAssets();

    inline bool isLoaded() const { return m_api != nullptr; }

    ~AssetManager() = default;

private:
    Mesh loadMesh(const std::filesystem::path&, gfx::GraphicAPI&);

    gfx::GraphicAPI* m_api = nullptr;
    std::filesystem::path m_baseDir;

    utils::Dictionary<std::filesystem::path, AssetID> m_registeredMeshes;

    utils::Dictionary<AssetID, Mesh> m_loadedMeshes;

public:
    AssetManager& operator = (const AssetManager&) = default;
    AssetManager& operator = (AssetManager&&)      = default;

    friend void to_json(nlohmann::json&, const AssetManager&);
    friend void from_json(const nlohmann::json&, AssetManager&);
};

}

#endif // ASSETMANAGER_HPP