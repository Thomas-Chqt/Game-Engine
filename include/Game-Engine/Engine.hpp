/*
 * ---------------------------------------------------
 * Engine.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:28:28
 * ---------------------------------------------------
 */

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Graphics/Event.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class Game;

class Engine
{
public:
    Engine(const Engine&) = delete;
    Engine(Engine&&)      = delete;

    static void init();
    static inline Engine& shared() { return *s_sharedInstance; }
    static inline void terminate() { s_sharedInstance.clear(); }

    virtual gfx::Window& mainWindow() = 0;

    virtual void runGame(utils::UniquePtr<Game>&&) = 0;
    virtual void terminateGame() = 0;

    virtual void editorForGame(utils::UniquePtr<Game>&&) = 0;

    virtual const utils::Set<int>& pressedKeys() = 0;

    virtual ~Engine() = default;

protected:
    Engine() = default;

    static inline utils::UniquePtr<Engine> s_sharedInstance;

public:
    Engine& operator = (const Engine&) = delete;
    Engine& operator = (Engine&&)      = delete;
};

}

#endif // ENGINE_HPP