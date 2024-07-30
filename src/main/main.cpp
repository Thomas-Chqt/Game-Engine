/*
 * ---------------------------------------------------
 * main.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:45:26
 * ---------------------------------------------------
 */

#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "UtilsCPP/UniquePtr.hpp"

extern utils::UniquePtr<GE::Game> createGame(int argv, char* argc[]);

int main(int argv, char* argc[])
{
    GE::Engine::init();

    GE::Engine::shared().runGame(createGame(argv, argc));

    GE::Engine::terminate();
}