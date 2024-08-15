/*
 * ---------------------------------------------------
 * Engine.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:28:28
 * ---------------------------------------------------
 */

#ifndef ENGINE_HPP
# define ENGINE_HPP

#include "Graphics/Event.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/Types.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class Game;

class Engine
{
public:
    Engine(const Engine&) = delete;
    Engine(Engine&&)      = delete;

    static inline void init() { s_sharedInstance = utils::UniquePtr<Engine>(new Engine()); }
    static inline Engine& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    inline gfx::Window& mainWindow() { return *m_mainWindow; }

    void runGame(utils::UniquePtr<Game>&&);
    inline void terminateGame() { m_running = false; }

    utils::Set<int> pressedKeys() { return m_pressedKeys; }

    ~Engine();

private:
    Engine();

    void onEvent(gfx::Event& event);

    static inline utils::UniquePtr<Engine> s_sharedInstance;

    utils::SharedPtr<gfx::Window> m_mainWindow;

    bool m_running = false;
    utils::UniquePtr<Game> m_runningGame;

    utils::Set<int> m_pressedKeys;

public:
    Engine& operator = (const Engine&) = delete;
    Engine& operator = (Engine&&)      = delete;
};

}

#endif // ENGINE_HPP