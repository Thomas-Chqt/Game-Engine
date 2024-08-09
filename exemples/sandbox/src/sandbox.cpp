/*
 * ---------------------------------------------------
 * sandbox.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:38:24
 * ---------------------------------------------------
 */

#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Game.hpp"
#include "Game-Engine/Scene.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <iostream>

class Player : public GE::ScriptableEntity
{
public:
    using GE::ScriptableEntity::ScriptableEntity;

    void onUpdate() override
    {
        std::cout << "On update" << std::endl;
    };
};

class Sandbox : public GE::Game
{
public:
    Sandbox()
    {
        m_scene.newScriptableEntity<Player>("player");
        m_activeScene = &m_scene;
    }

    void onWindowCloseEvent() override
    {
        std::cout << "Ending game" << std::endl;
        GE::Engine::terminateGame();
    }

private:
    GE::Scene m_scene;
};

int main()
{
    GE::Engine::init();

    GE::Engine::runGame(utils::makeUnique<Sandbox>().staticCast<GE::Game>());

    GE::Engine::terminate();
}
