/*
 * ---------------------------------------------------
 * Engine.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/31 12:46:07
 * ---------------------------------------------------
 */

#ifndef ENGINE_HPP
#define ENGINE_HPP

#include "Game-Engine/Game.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class Engine
{
public:
    Engine(const Engine&) = delete;
    Engine(Engine&&)      = delete;
    
    inline static void init() { s_sharedInstance = utils::UniquePtr<Engine>(new Engine()); }
    inline static Engine& shared() { return *s_sharedInstance; }
    inline static void terminate() { s_sharedInstance.clear(); }

    void openProject(const utils::String& filepath);

    void run();

    ~Engine();

private:
    Engine();

    void onEvent(gfx::Event&);
    void onImGuiRender();

    static utils::UniquePtr<Engine> s_sharedInstance;

    utils::SharedPtr<gfx::Window> m_window;
    Renderer m_renderer;

    bool m_running = false;

    utils::UniquePtr<Game> m_game;

public:
    Engine& operator = (const Engine&) = delete;
    Engine& operator = (Engine&&)      = delete;
};

}

#endif // ENGINE_HPP