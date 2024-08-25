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
#include "Game-Engine/Engine.hpp"
#include "Graphics/Event.hpp"
#include "Game-Engine/InputContext.hpp"

namespace GE
{

class Game
{
public:
    Game(const Game&) = delete;
    Game(Game&&)      = delete;

    inline ECSWorld& activeScene() { return *m_activeScene; }
    inline InputContext& inputContext() { return m_inputContext; }

    virtual void onWindowResizeEvent(gfx::WindowResizeEvent&) {}
    inline virtual void onWindowRequestCloseEvent(gfx::WindowRequestCloseEvent&) { Engine::shared().terminateGame(); }

    inline virtual void onSetup() {}
    inline virtual void onUpdate() {}

    virtual ~Game() = default;

protected:
    Game() = default;

    InputContext m_inputContext;

    ECSWorld m_defaultScene;
    ECSWorld* m_activeScene = &m_defaultScene;
};

}

#endif // GAME_HPP