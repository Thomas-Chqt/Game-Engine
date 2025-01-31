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
#include "ECS/ECSView.hpp"
#include "ECS/Components.hpp"
#include "InputManager/InputContext.hpp"

namespace GE
{

Game::Game(const Descriptor& descriptor)
{
    m_scenes = descriptor.scenes;
    m_inputContext = descriptor.inputContext;
    m_graphicAPI = descriptor.graphicAPI;
    m_baseDir = descriptor.baseDir;
    m_makeScriptInstance = descriptor.makeScriptInstance;
}

Scene& Game::activeScene()
{
    assert(m_activeScene != nullptr);
    assert(m_activeScene->assetManager().isLoaded());
    return *m_activeScene;
}

void Game::setActiveScene(const utils::String& name)
{
    if (m_activeScene)
    {
        assert(m_activeScene->assetManager().isLoaded());
        if (m_activeScene->name() == name)
            return;
        ECSView<ScriptComponent>(m_activeScene->ecsWorld()).onEach([&](Entity, ScriptComponent& scriptComponent) {
            scriptComponent.instance.clear();
        });
        m_activeScene->assetManager().unloadAssets();
    }

    auto it = m_scenes.find(name);
    assert(it != m_scenes.end());
    m_activeScene = &*it;

    if (m_activeScene->assetManager().isLoaded() == false)
        m_activeScene->assetManager().loadAssets(*m_graphicAPI, m_baseDir);

    if (m_makeScriptInstance != nullptr)
    {
        ECSView<ScriptComponent>(m_activeScene->ecsWorld()).onEach([&](Entity entt, ScriptComponent& scriptComponent) {
            scriptComponent.instance = utils::SharedPtr<Script>(m_makeScriptInstance(scriptComponent.name, entt, *this));
            assert(scriptComponent.instance);
        });
    }
}

}