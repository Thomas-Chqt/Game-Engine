/*
 * ---------------------------------------------------
 * Scene.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/31 16:52:15
 * ---------------------------------------------------
 */

#ifndef SCENE_HPP
#define SCENE_HPP

#include "AssetManager.hpp"
#include "ECS/Components.hpp"
#include "ECS/ECSWorld.hpp"
#include "ECS/Entity.hpp"
#include "ECS/ECSView.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

class Scene
{
public:
    Scene()             = default;
    Scene(const Scene&) = delete;
    Scene(Scene&&)      = default;

    Entity newEntity(const utils::String& name = "No name");
    inline void forEachNamedEntity(const utils::Func<void(Entity, NameComponent&)> &f) { ECSView<NameComponent>(m_ecsWorld).onEach(f); }

    inline const Entity& activeCamera() { return m_activeCamera; }
    inline void setActiveCamera(Entity e) { m_activeCamera = e; }

    inline AssetManager& assetManager() { return m_assetManager; }

    inline void load(gfx::GraphicAPI& api) { m_assetManager.loadAssets(api); }
    inline void unload() { m_assetManager.unloadAssets(); }

    void submitMeshesForRendering(Renderer&);
    void submitLightsForRendering(Renderer&);
    void submitForRendering(Renderer&);

private:
    ECSWorld m_ecsWorld;
    Entity m_activeCamera;
    AssetManager m_assetManager;

public:
    Scene& operator = (const Scene&) = delete;
    Scene& operator = (Scene&&)      = default;
};

}

#endif // SCENE_HPP