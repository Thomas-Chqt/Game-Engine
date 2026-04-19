/*
 * ---------------------------------------------------
 * Game.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef GAME_HPP
#define GAME_HPP

#include "Game-Engine/InputContext.hpp"
#include "Game-Engine/Scene.hpp"
#include "Game-Engine/AssetManager.hpp"

#include <map>
#include <string>

namespace GE
{

class Game
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

    Game(AssetManager*, const Descriptor&);

    auto& activeScene(this auto&& self) { return *self.m_activeScene; }
    auto& inputContext(this auto&& self) { return self.m_inputContext; }
    void setActiveScene(const std::string& name) { m_activeScene = &m_scenes.at(name); }

private:
    std::map<std::string, Scene> m_scenes;
    Scene* m_activeScene = nullptr;
    InputContext m_inputContext;

public:
    Game& operator = (const Game&) = delete;
    Game& operator = (Game&&) = delete;
};

}

#endif
