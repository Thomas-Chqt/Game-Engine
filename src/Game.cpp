/*
 * ---------------------------------------------------
 * Game.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/Game.hpp"
#include "Game-Engine/ScriptLibrary.hpp"
#include "Game-Engine/ECSView.hpp"
#include "Game-Engine/Scene.hpp"

#include <algorithm>
#include <cassert>
#include <future>
#include <memory>
#include <utility>

namespace GE
{

namespace
{

void setupSceneScripts(Game& game, Scene& scene, const ScriptLibrary& scriptLibrary)
{
    for (Entity entity : scene.ecsWorld() | ECSView<ScriptComponent>() | std::views::transform([&](auto id) { return Entity{ &scene.ecsWorld(), id }; }))
    {
        ScriptComponent& scriptComponent = entity.get<ScriptComponent>();
        scriptComponent.instance = scriptLibrary.makeScriptInstance(scriptComponent.name);
        assert(scriptComponent.instance);
        for (const std::string& parameterName : scriptLibrary.listScriptParameterNames(scriptComponent.name))
        {
            auto it = scriptComponent.parameters.find(parameterName);
            scriptLibrary.setScriptParameter(
                scriptComponent.name,
                parameterName,
                *scriptComponent.instance,
                it == scriptComponent.parameters.end() ? scriptLibrary.getScriptDefaultParameterValue(scriptComponent.name, parameterName) : it->second
            );
        }
        scriptComponent.instance->setup(entity, game);
    }
}

void tearDownSceneScripts(Game& game, Scene& scene)
{
    for (Entity entity : scene.ecsWorld() | ECSView<ScriptComponent>() | std::views::transform([&](auto id) { return Entity{ &scene.ecsWorld(), id }; }))
    {
        ScriptComponent& scriptComponent = entity.get<ScriptComponent>();
        assert(scriptComponent.instance);
        scriptComponent.instance->teardown(entity, game);
        scriptComponent.instance.reset();
    }
}

}

Game::Game(const Descriptor& desc)
    : m_assetManager(desc.assetManager)
    , m_inputContext(desc.inputContext)
    , m_sceneDescriptors(desc.scenes)
    , m_scriptLibrary(desc.scriptLibrary)
{
    assert(m_assetManager);

    std::ranges::sort(m_sceneDescriptors, std::ranges::less{}, &Scene::Descriptor::name);

    loadScene(desc.startSceneName).get();
    setActiveScene(desc.startSceneName);
}

std::shared_future<void> Game::loadScene(const std::string& name)
{
    assert(m_loadedScenes.contains(name) == false);

    auto descriptorIt = std::ranges::lower_bound(m_sceneDescriptors, name, std::ranges::less{}, &Scene::Descriptor::name);
    assert(descriptorIt != m_sceneDescriptors.end());
    assert(descriptorIt->name == name);

    auto [it, inserted] = m_loadedScenes.try_emplace(name, m_assetManager, *descriptorIt);
    assert(inserted);

    return it->second.loadFuture();
}

void Game::unloadScene(const std::string& name)
{
    auto it = m_loadedScenes.find(name);
    assert(it != m_loadedScenes.end());
    assert(m_activeScene != &it->second);
    m_loadedScenes.erase(it);
}

bool Game::isSceneLoaded(const std::string& name) const
{
    auto it = m_loadedScenes.find(name);
    return it != m_loadedScenes.end() && it->second.loadFuture().wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

void Game::setActiveScene(const std::string& name)
{
    if (m_activeScene && m_scriptLibrary)
        tearDownSceneScripts(*this, *m_activeScene);

    auto it = m_loadedScenes.find(name);
    assert(it != m_loadedScenes.end());
    m_activeScene = &it->second;

    if (m_scriptLibrary)
        setupSceneScripts(*this, *m_activeScene, *m_scriptLibrary);
}

Game::~Game()
{
    assert(m_activeScene);
    if (m_scriptLibrary)
        tearDownSceneScripts(*this, *m_activeScene);
    m_activeScene = nullptr;
    m_inputContext.clearAllInputCallbacks();
}

}
