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
#include "ECS/ECSWorld.hpp"
#include "ECS/Entity.hpp"

namespace GE
{

class Scene
{
public:
    Scene()             = default;
    Scene(const Scene&) = delete;
    Scene(Scene&&)      = default;

    inline ECSWorld& ecsWorld() { return m_ecsWorld; }

    inline const Entity& activeCamera() { return m_activeCamera; }
    inline void setActiveCamera(Entity e) { m_activeCamera = e; }

    inline AssetManager& assetManager() { return m_assetManager; }

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