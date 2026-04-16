/*
 * ---------------------------------------------------
 * Game.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "Game-Engine/Game.hpp"
#include "Game-Engine/Scene.hpp"
#include "Game-Engine/AssetManager.hpp"

#include <cassert>
#include <ranges>
#include <utility>

namespace GE
{

Game::Game(AssetManager* assetManager, const Descriptor& desc)
    : m_scenes(std::from_range, desc.scenes | std::views::transform([&](const auto& sceneDesc){ return std::make_pair(sceneDesc.first, Scene(assetManager, sceneDesc.second)); }))
    , m_activeScene(&m_scenes.at(desc.activeScene))
    , m_inputContext(desc.inputContext)
{
}

void Game::switchActiveScene(const std::string& name)
{
    Scene* newActiveScene = &m_scenes.at(name);
    /*
    if (newActiveScene->isLoaded() == false) {
        newActiveScene->load();
        block until scene is loaded
    }
    */
}

}
