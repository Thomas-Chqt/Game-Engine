/*
 * ---------------------------------------------------
 * Engine.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/06 20:36:10
 * ---------------------------------------------------
 */

#include "Game-Engine/Engine.hpp"
#include "Engine/EngineIntern.hpp"

namespace GE::Engine
{

void init()
{
    EngineIntern::init();
}

void runGame(utils::UniquePtr<Game>&& game)
{
    EngineIntern::shared().runGame(std::move(game));
}

void terminateGame()
{
    EngineIntern::shared().terminateGame();
}

utils::Set<int> pressedKeys()
{
    return EngineIntern::shared().pressedKeys();
}

void terminate()
{
    EngineIntern::terminate();
}

}