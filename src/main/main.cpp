/*
 * ---------------------------------------------------
 * main.cpp
 *
 * Author: Thomas Choquet <thomas.publique@icloud.com>
 * Date: 2024/06/20 16:45:26
 * ---------------------------------------------------
 */

#include "Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "Graphics/Platform.hpp"
#include "UtilsCPP/UniquePtr.hpp"

extern utils::UniquePtr<GE::Game> createGame(int argv, char* argc[]);

int main(int argv, char* argc[])
{
    gfx::Platform::init();
    GE::EngineSingleton::init();

    GE::EngineSingleton::shared().runGame(createGame(argv, argc));

    GE::EngineSingleton::terminate();
    gfx::Platform::terminate();
}