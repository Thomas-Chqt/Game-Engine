/*
 * ---------------------------------------------------
 * sandbox.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:38:24
 * ---------------------------------------------------
 */

#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <iostream>

class Sandbox : public GE::Game
{
public:
    Sandbox()
    {
    }

    void onWindowCloseEvent() override
    {
        std::cout << "Ending game" << std::endl;
        GE::Engine::terminateGame();
    }

    ~Sandbox() = default;
};

utils::UniquePtr<GE::Game> createGame(int argv, char* argc[])
{
    return utils::UniquePtr<GE::Game>(new Sandbox);
}
