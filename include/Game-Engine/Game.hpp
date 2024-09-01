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
#include "Graphics/Event.hpp"

namespace GE
{

class Game
{
public:
    Game(const Game&) = delete;
    Game(Game&&)      = delete;

    inline ECSWorld& activeScene() { return *m_activeScene; }

    inline virtual void onSetup() {}
    inline virtual void onUpdate() {}

    virtual void onEvent(gfx::Event&);
    virtual void onImGuiRender() {};

    virtual ~Game() = default;

protected:
    Game() = default;

    ECSWorld m_defaultScene;
    ECSWorld* m_activeScene = &m_defaultScene;
};

}

#endif // GAME_HPP