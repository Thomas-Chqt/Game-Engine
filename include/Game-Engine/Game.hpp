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
#include "UtilsCPP/Types.hpp"

namespace GE
{

class Game
{
private:
    friend class EngineSingleton;

public:
    Game(const Game&) = delete;
    Game(Game&&)      = delete;

    virtual void onWindowCloseEvent();

    virtual ~Game() = default;

protected:
    Game() = default;

    utils::uint32 m_windowWidth = 800;
    utils::uint32 m_windowHeight = 600;

    ECSWorld m_defaultECSWorld;
    ECSWorld* m_activeECSWorld = &m_defaultECSWorld;
};

}

#endif // GAME_HPP