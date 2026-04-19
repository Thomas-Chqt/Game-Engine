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
#include <utility>

namespace GE
{

Game::Game(AssetManager* assetManager, const Descriptor& desc)
    : m_activeScene(nullptr)
    , m_inputContext(desc.inputContext)
{
    for (const auto& [name, sceneDesc] : desc.scenes)
        m_scenes.insert(std::make_pair(name, Scene(assetManager, sceneDesc)));

    m_activeScene = &m_scenes.at(desc.activeScene);
}

void Game::switchActiveScene(const std::string& name)
{
    m_activeScene = &m_scenes.at(name);
    /*
    if (newActiveScene->isLoaded() == false) {
        newActiveScene->load();
        block until scene is loaded
    }
    */
}

}
