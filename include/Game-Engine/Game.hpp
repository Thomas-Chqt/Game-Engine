/*
 * ---------------------------------------------------
 * Game.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:57:06
 * ---------------------------------------------------
 */

#ifndef GAME_HPP
# define GAME_HPP

#include "Game-Engine/ECSWorld.hpp"

namespace GE
{

class Game
{
private:
    friend class InternalEngine;

public:
    Game(const Game&) = delete;
    Game(Game&&)      = delete;

    virtual void onWindowRequestCloseEvent();
    virtual void onKeyDownEvent(int keyCode, bool isRepeat) = 0;

    virtual ~Game() = default;

protected:
    Game() = default;

    ECSWorld m_defaultECSWorld;
    ECSWorld* m_activeECSWorld = &m_defaultECSWorld;
};

}

#endif // GAME_HPP