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

Game::Game(const utils::Set<Scene>& scene, const utils::String& startScene)
    : m_scenes(scene), m_activeScene(&*m_scenes.find(startScene))
{
}

void Game::start(gfx::GraphicAPI& api, const std::filesystem::path& baseDir, Script *(*makeScriptInstanceFn)(char *))
{
    m_api = &api;
    m_baseDir = baseDir;
    m_makeScriptInstanceFn = makeScriptInstanceFn;

    assert(m_activeScene);
    m_activeScene->load(api, baseDir, makeScriptInstanceFn);

    m_isRunning = true;
}

void Game::stop()
{
    m_scenes.clear();
    m_isRunning = false;
}

void Game::setActiveScene(const utils::String& name)
{
    assert(m_activeScene);
    m_activeScene->unload();

    m_activeScene = &*m_scenes.find(name);
    m_activeScene->load(*m_api, m_baseDir, m_makeScriptInstanceFn);
}

}