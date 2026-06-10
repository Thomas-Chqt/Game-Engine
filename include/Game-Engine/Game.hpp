/*
 * ---------------------------------------------------
 * Game.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef GAME_HPP
#define GAME_HPP

#include "Game-Engine/Export.hpp"
#include "Game-Engine/InputContext.hpp"
#include "Game-Engine/Scene.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/ScriptLibrary.hpp"

#include <future>
#include <map>
#include <string>
#include <vector>

namespace GE
{

class GE_API Game
{
public:
    struct Descriptor
    {
        AssetManager* assetManager;
        InputContext inputContext;
        std::vector<Scene::Descriptor> scenes;
        std::string startSceneName;
        const ScriptLibrary* scriptLibrary;
    };

public:
    Game() = delete;
    Game(const Game&) = delete;
    Game(Game&&) = delete;

    Game(const Descriptor&);

    auto& inputContext(this auto&& self) { return self.m_inputContext; }

    std::shared_future<void> loadScene(const std::string& name);
    void unloadScene(const std::string& name);

    auto& activeScene(this auto&& self) { return *self.m_activeScene; }
    void setActiveScene(const std::string& name);

    bool isSceneLoaded(const std::string& name) const;

    ~Game();

private:
    AssetManager* m_assetManager = nullptr;
    InputContext m_inputContext;
    std::vector<Scene::Descriptor> m_sceneDescriptors;
    std::map<std::string, Scene> m_loadedScenes;
    const ScriptLibrary* m_scriptLibrary = nullptr;

    Scene* m_activeScene = nullptr;

public:
    Game& operator = (const Game&) = delete;
    Game& operator = (Game&&) = delete;
};

}

#endif
