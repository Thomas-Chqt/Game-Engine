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

#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Entity.hpp"

#include <string>

namespace GE
{

class Scene
{
public:
    Scene() = default;
    Scene(const Scene&) = default;
    Scene(Scene&&) = default;

    Scene(const std::string& name);

    inline const std::string& name() const { return m_name; }
    inline void setName(const std::string& s) { m_name = s; }

    inline auto& ecsWorld(this auto&& self) { return self.m_ecsWorld; }

    auto activeCamera(this auto&& self)
    {
        if (self.m_activeCamera == INVALID_ENTITY_ID)
            return const_Entity();
        return basic_entity(&self.m_ecsWorld, self.m_activeCamera);
    }

    void setActiveCamera(const Entity& e);

    Entity newEntity(const std::string& name);

    ~Scene() = default;

private:
    std::string m_name;
    ECSWorld m_ecsWorld;
    ECSWorld::EntityID m_activeCamera = INVALID_ENTITY_ID;

public:
    Scene& operator=(const Scene&) = default;
    Scene& operator=(Scene&&) = default;
};

} // namespace GE

#endif // SCENE_HPP
