/*
 * ---------------------------------------------------
 * Game.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/14 16:40:46
 * ---------------------------------------------------
 */

#include "Game-Engine/Game.hpp"
#include "Game-Engine/Engine.hpp"

namespace GE
{

void Game::onWindowRequestCloseEvent(gfx::WindowRequestCloseEvent&)
{
    Engine::shared().terminateGame();
}

}