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

struct Scene
{
    ECSWorld ecsWorld;
    Entity activeCamera;
    AssetManager assetManager;
};

}

#endif // SCENE_HPP