/*
 * ---------------------------------------------------
 * Scene.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/07 17:29:26
 * ---------------------------------------------------
 */

#ifndef SCENE_HPP
#define SCENE_HPP

#include "ECSWorld.hpp"
#include "Entity.hpp"
#include "Game-Engine/Components.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class Scene
{
public:
    Scene()             = default;
    Scene(const Scene&) = delete;
    Scene(Scene&&)      = delete;
    
    Entity newEntity(const utils::String& name = "No name");

    template<typename T>
    Entity newScriptableEntity(const utils::String& name = "No name");

    void onUpdate();

    ~Scene() = default;

private:
    ECSWorld m_world;

public:
    Scene& operator = (const Scene&) = delete;
    Scene& operator = (Scene&&)      = delete;
};


template<typename T>
Entity Scene::newScriptableEntity(const utils::String& name)
{
    Entity newEntt = newEntity(name);
    ScriptComponent& comp = newEntt.emplace<ScriptComponent>();
    comp.instance = utils::makeUnique<T>(newEntt).template staticCast<ScriptableEntity>();
    return newEntt;
}

}

#endif // SCENE_HPP