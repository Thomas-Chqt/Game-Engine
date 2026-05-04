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
#include "Game-Engine/AssetManager.hpp"

#include <cassert>
#include <ranges>
#include <utility>

namespace GE
{

namespace
{

void setupScene(Game& game, Scene& scene, const ScriptLibrary* scriptLibrary)
{
    scene.load();
    if (scriptLibrary)
    {
        for (Entity entity : scene.ecsWorld() | ECSView<ScriptComponent>() | std::views::transform([&](auto id) { return Entity{ &scene.ecsWorld(), id }; }))
        {
            ScriptComponent& scriptComponent = entity.get<ScriptComponent>();
            scriptComponent.instance = scriptLibrary->makeScriptInstance(scriptComponent.name);
            assert(scriptComponent.instance);
            for (const std::string& parameterName : scriptLibrary->listScriptParameterNames(scriptComponent.name))
            {
                auto it = scriptComponent.parameters.find(parameterName);
                scriptLibrary->setScriptParameter(
                    scriptComponent.name,
                    parameterName,
                    *scriptComponent.instance,
                    it == scriptComponent.parameters.end() ? scriptLibrary->getScriptDefaultParameterValue(scriptComponent.name, parameterName) : it->second
                );
            }
            scriptComponent.instance->setup(entity, game);
        }
    }
}

void tearDownScene(Game& game, Scene& scene)
{
    for (Entity entity : scene.ecsWorld() | ECSView<ScriptComponent>() | std::views::transform([&](auto id) { return Entity{ &scene.ecsWorld(), id }; }))
    {
        ScriptComponent& scriptComponent = entity.get<ScriptComponent>();
        if (scriptComponent.instance != nullptr)
            scriptComponent.instance->teardown(entity, game);
        scriptComponent.instance.reset();
    }
    scene.unload();
}

}

Game::Game(AssetManager* assetManager, const ScriptLibrary* scriptLibrary, const Descriptor& descriptor)
    : m_scenes(descriptor.scenes
               | std::views::transform([&](const auto& sceneDesc) {
                     return std::make_pair(sceneDesc.first, Scene(assetManager, sceneDesc.second));
                 })
               | std::ranges::to<std::map<std::string, Scene>>())
    , m_inputContext(descriptor.inputContext)
    , m_scriptLibrary(scriptLibrary)
{
    setActiveScene(descriptor.activeScene);
}

void Game::setActiveScene(const std::string& name)
{
    if (m_activeScene)
        tearDownScene(*this, *m_activeScene);
    m_activeScene = &m_scenes.at(name);
    setupScene(*this, *m_activeScene, m_scriptLibrary);
}

Game::~Game()
{
    assert(m_activeScene);
    tearDownScene(*this, *m_activeScene);
    m_inputContext.clearAllInputCallbacks();
}

}
