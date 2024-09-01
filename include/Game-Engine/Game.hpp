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
#include "Game-Engine/InputContext.hpp"
#include "Game-Engine/Scene.hpp"
#include "UtilsCPP/Dictionary.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

class Game
{
public:
    Game(const Game&) = delete;
    Game(Game&&)      = delete;

    inline Scene& activeScene() { return *m_activeScene; }

    inline void onInputEvent(gfx::InputEvent& event) { m_inputContext.onInputEvent(event); }

    virtual void onWindowResizeEvent(gfx::WindowResizeEvent&) {}
    inline virtual void onWindowRequestCloseEvent(gfx::WindowRequestCloseEvent&) {}

    inline virtual void onSetup() {}
    inline virtual void onUpdate() {}

    virtual ~Game() = default;

protected:
    Game() = default;

    InputContext m_inputContext;

    ECSWorld m_defaultScene;

private:
    Scene* m_activeScene;
    utils::Dictionary<utils::String, Scene> m_scenes;
};

}

#endif // GAME_HPP