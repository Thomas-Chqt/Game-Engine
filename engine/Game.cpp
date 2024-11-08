/*
 * ---------------------------------------------------
 * Game.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/07 14:45:12
 * ---------------------------------------------------
 */

#include "Game.hpp"
#include <cassert>
#include <dlfcn.h>

namespace GE
{

Game::Game(const utils::Set<Scene>& scenes)
    : m_scenes(scenes)
{
}

void Game::start(gfx::GraphicAPI& api, const std::filesystem::path& baseDir, MakeScriptInstanceFn makeScriptInstance)
{
    m_api = &api;
    m_baseDir = baseDir;
    m_makeScriptInstance = makeScriptInstance;

    m_isRunning = true;
}

void Game::stop()
{
    assert(m_activeScene != nullptr);
    assert(m_activeScene->isLoaded());
    
    m_activeScene->unload();
    m_activeScene = nullptr;
    m_isRunning = false;
}

void Game::setActiveScene(const utils::String& name)
{
    if (m_activeScene)
    {
        assert(m_activeScene->isLoaded());
        m_activeScene->unload();
    }
    auto it = m_scenes.find(name);
    assert(it != m_scenes.end());
    m_activeScene = &*it;
    m_activeScene->load(*m_api, m_baseDir, m_makeScriptInstance, *this);
}

}