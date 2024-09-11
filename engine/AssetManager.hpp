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
#include "UtilsCPP/Set.hpp"
// #include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

class AssetManager
{
public:
    AssetManager()                    = default;
    AssetManager(const AssetManager&) = delete;
    AssetManager(AssetManager&&)      = default;

    inline void registerMesh(const utils::String& filepath) { m_registeredMeshes.insert(filepath); }
    // inline void registerTexture(const utils::String& filepath) { m_registeredTexture.insert(filepath); }
    // void registerMaterial(const utils::String& filepath);

    void loadAssets(gfx::GraphicAPI&);
    void unloadAssets();

    inline const utils::Dictionary<utils::String, Mesh>& loadedMeshes() { return m_loadedMeshes; }

    ~AssetManager() = default;

private:
    Mesh loadMesh(const utils::String& filepath, gfx::GraphicAPI&);
    // utils::SharedPtr<gfx::Texture> loadTexture(const utils::String& filepath, gfx::GraphicAPI&);
    // Material loadMaterial(const utils::String& filepath, gfx::GraphicAPI&);

    utils::Set<utils::String> m_registeredMeshes;
    // utils::Set<utils::String> m_registeredTexture;
    // utils::Set<utils::String> m_registeredMaterial;

    utils::Dictionary<utils::String, Mesh> m_loadedMeshes;
    // utils::Dictionary<utils::String, utils::SharedPtr<gfx::Texture>> m_loadedTextures;
    // utils::Dictionary<utils::String, Material> m_loadedMaterials;

public:
    AssetManager& operator = (const AssetManager&) = delete;
    AssetManager& operator = (AssetManager&&)      = default;

};

}

#endif // ASSETMANAGER_HPP