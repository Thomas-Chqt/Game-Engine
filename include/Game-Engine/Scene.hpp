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
#include "ECSWorld.hpp"

namespace GE
{

class Scene
{
public:
    Scene()             = default;
    Scene(const Scene&) = delete;
    Scene(Scene&&)      = default;

    ~Scene() = default;

private:
    ECSWorld m_ecsWorld;
    AssetManager m_assets;
    
public:
    Scene& operator = (const Scene&) = delete;
    Scene& operator = (Scene&&)      = default;
};

}

#endif // SCENE_HPP