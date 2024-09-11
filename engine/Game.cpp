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

void Game::deleteScene(const utils::String& name)
{
    assert(name != m_startSceneName);
    m_scenes.remove(name);
}

void Game::setStartScene(const utils::String& name)
{
    assert(m_scenes.contain(name));
    m_startSceneName = name;
}

}