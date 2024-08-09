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

#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Scene.hpp"
namespace GE
{

class Game
{
public:
    Game(const Game&) = delete;
    Game(Game&&)      = delete;

    inline Scene& activeScene() { return *m_activeScene; }

    inline virtual void onWindowCloseEvent() { Engine::terminateGame(); }

    virtual ~Game() = default;

protected:
    Game() = default;

    Scene m_defaultScene;
    Scene* m_activeScene = &m_defaultScene;
};

}

#endif // GAME_HPP