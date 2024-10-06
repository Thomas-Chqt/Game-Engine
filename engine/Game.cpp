/*
 * ---------------------------------------------------
 * Game.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/08 12:34:02
 * ---------------------------------------------------
 */

#include "Game.hpp"
#include <cassert>

namespace GE
{

Game::Game(utils::Set<Scene>& scenes)
    : m_scenes(scenes)
{
}

void Game::deleteScene(const utils::String& name)
{
    assert(name != m_startScene->name());
    m_scenes.remove(m_scenes.find(name));
}

void Game::setStartScene(const utils::String& name)
{
    assert(m_scenes.contain(name));
    m_startScene = &*m_scenes.find(name);
}

}