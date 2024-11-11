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
#include "UtilsCPP/String.hpp"
#include <nlohmann/json.hpp>

namespace GE
{

class Scene
{
public:
    Scene()             = default;
    Scene(const Scene&) = default;
    Scene(Scene&&)      = default;

    Scene(const utils::String& name);
    
    inline const utils::String& name() const { return m_name; }
    inline void setName(const utils::String& s) { m_name = s; }

    inline ECSWorld& ecsWorld() { return m_ecsWorld; }
    inline const ECSWorld& ecsWorld() const { return const_cast<Scene*>(this)->ecsWorld(); }

    Entity activeCamera();
    const Entity activeCamera() const { return const_cast<Scene*>(this)->activeCamera(); };
    
    void setActiveCamera(const Entity& e);

    inline AssetManager& assetManager() { return m_assetManager; }
    inline const AssetManager& assetManager() const { return const_cast<Scene*>(this)->assetManager(); }

    Entity newEntity(const utils::String& name);
    
    ~Scene() = default;

private:
    utils::String m_name;
    ECSWorld m_ecsWorld;
    ECSWorld::EntityID m_activeCamera = INVALID_ENTITY_ID;
    AssetManager m_assetManager;

public:
    Scene& operator = (const Scene&) = default;
    Scene& operator = (Scene&&)      = default;

    inline bool operator  < (const Scene& rhs) const { return m_name  < rhs.m_name; }
    inline bool operator == (const Scene& rhs) const { return m_name == rhs.m_name; }

    inline bool operator  < (const utils::String& name) const { return m_name  < m_name; }
    inline bool operator == (const utils::String& name) const { return m_name == m_name; }

    friend void to_json(nlohmann::json&, const Scene&);
    friend void from_json(const nlohmann::json&, Scene&);
};

}

#endif // SCENE_HPP