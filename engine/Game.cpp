/*
 * ---------------------------------------------------
 * Game.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/08 12:34:02
 * ---------------------------------------------------
 */

#include "Game.hpp"
#include "Scene.hpp"
#include <cassert>

namespace GE
{

Game::Game(utils::Set<Scene>& scenes)
    : m_scenes(scenes)
{
}

void Game::setActiveScene(Scene& scene)
{
    assert(scene.isLoaded());
    m_activeScene = &scene;
}

}