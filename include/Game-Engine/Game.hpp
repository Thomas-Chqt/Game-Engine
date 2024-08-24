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
#include "UtilsCPP/Types.hpp"

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

    virtual void onWindowResizeEvent(gfx::WindowResizeEvent&) {}
    virtual void onWindowRequestCloseEvent(gfx::WindowRequestCloseEvent&);
    virtual void onImGuiRender() {};

    virtual ~Game() = default;

protected:
    Game() = default;

    utils::uint32 m_windowWidth = 800;
    utils::uint32 m_windowHeight = 600;

    ECSWorld m_defaultScene;
    ECSWorld* m_activeScene = &m_defaultScene;
};

}

#endif // GAME_HPP