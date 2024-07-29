/*
 * ---------------------------------------------------
 * Game.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 17:53:32
 * ---------------------------------------------------
 */

#include "Game-Engine/Game.hpp"
#include "Game-Engine/Engine.hpp"

namespace GE
{

void Game::onWindowCloseEvent()
{
    Engine::terminateGame();
}

}