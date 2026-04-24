/*
 * ---------------------------------------------------
 * Game.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/Game.hpp"
#include "Game-Engine/ScriptLibraryManager.hpp"
#include "Game-Engine/ECSView.hpp"
#include "Game-Engine/Scene.hpp"
#include "Game-Engine/AssetManager.hpp"

#include <format>
#include <cassert>
#include <ranges>
#include <stdexcept>
#include <utility>

namespace GE
{

namespace
{

void setupScene(
    Game& game,
    Scene& scene,
    const ScriptLibraryMakeScriptInstance& makeScriptInstance,
    const ScriptLibraryListScriptParameters& listScriptParameters
)
{
    scene.load();
    for (Entity entity : scene.ecsWorld()
                             | ECSView<ScriptComponent>()
                             | std::views::transform([&](auto id) { return Entity{ &scene.ecsWorld(), id }; }))
    {
        if (!makeScriptInstance || !listScriptParameters)
            throw std::runtime_error(std::format("unable to create script '{}' without a loaded script library", entity.get<ScriptComponent>().name));
        ScriptComponent& scriptComponent = entity.get<ScriptComponent>();
        scriptComponent.instance = makeScriptInstance(scriptComponent.name);
        assert(scriptComponent.instance);
        for (const ScriptParameterDescriptor& parameter : listScriptParameters(scriptComponent.name))
        {
            auto it = scriptComponent.parameters.find(parameter.name);
            parameter.set(*scriptComponent.instance, it == scriptComponent.parameters.end() ? parameter.defaultValue : it->second);
        }
        scriptComponent.instance->setup(entity, game);
    }
}

void tearDownScene(Game& game, Scene& scene)
{
    for (Entity entity : scene.ecsWorld()
                             | ECSView<ScriptComponent>()
                             | std::views::transform([&](auto id) { return Entity{ &scene.ecsWorld(), id }; }))
    {
        ScriptComponent& scriptComponent = entity.get<ScriptComponent>();
        if (scriptComponent.instance != nullptr)
            scriptComponent.instance->teardown(entity, game);
        scriptComponent.instance.reset();
    }
    scene.unload();
}

}

Game::Game(
    AssetManager* assetManager,
    ScriptLibraryMakeScriptInstance makeScriptInstance,
    ScriptLibraryListScriptParameters listScriptParameters,
    const Descriptor& descriptor
)
    : m_makeScriptInstance(std::move(makeScriptInstance))
    , m_listScriptParameters(std::move(listScriptParameters))
    , m_scenes(std::from_range, descriptor.scenes | std::views::transform([&](const auto& sceneDesc){ return std::make_pair(sceneDesc.first, Scene(assetManager, sceneDesc.second)); }))
    , m_inputContext(descriptor.inputContext)
{
    setActiveScene(descriptor.activeScene);
}

void Game::setActiveScene(const std::string& name)
{
    if (m_activeScene)
        tearDownScene(*this, *m_activeScene);
    m_activeScene = &m_scenes.at(name);
    setupScene(*this, *m_activeScene, m_makeScriptInstance, m_listScriptParameters);
}

Game::~Game()
{
    assert(m_activeScene);
    tearDownScene(*this, *m_activeScene);
}

}
