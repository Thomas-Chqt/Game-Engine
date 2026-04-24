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
#include "Game-Engine/ScriptLibraryManager.hpp"

#include <map>
#include <string>

namespace GE
{

class GE_API Game
{
public:
    struct Descriptor
    {
        std::map<std::string, Scene::Descriptor> scenes;
        std::string activeScene;
        InputContext inputContext;
    };

public:
    Game() = delete;
    Game(const Game&) = delete;
    Game(Game&&) = delete;

    Game(
        AssetManager* assetManager,
        ScriptLibraryMakeScriptInstance makeScriptInstance,
        ScriptLibraryListScriptParameters listScriptParameters,
        const Descriptor& descriptor
    );

    auto& activeScene(this auto&& self) { return *self.m_activeScene; }
    void setActiveScene(const std::string& name);

    auto& inputContext(this auto&& self) { return self.m_inputContext; }

    ~Game();

private:
    ScriptLibraryMakeScriptInstance m_makeScriptInstance;
    ScriptLibraryListScriptParameters m_listScriptParameters;
    std::map<std::string, Scene> m_scenes;
    Scene* m_activeScene = nullptr;
    InputContext m_inputContext;

public:
    Game& operator = (const Game&) = delete;
    Game& operator = (Game&&) = delete;
};

}

#endif
