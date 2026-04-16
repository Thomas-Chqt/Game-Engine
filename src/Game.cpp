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

namespace GE
{

Game::Game(AssetManager* assetManager, const Descriptor& desc)
    : m_inputContext(desc.inputContext)
{
    for (const auto& sceneDesc : desc.scenes)
        m_scenes.emplace(sceneDesc.name, Scene(assetManager, sceneDesc));

    m_activeScene = &m_scenes.at(desc.activeScene);
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
