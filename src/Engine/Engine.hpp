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

#include "Application/Application.hpp"
#include "Graphics/Window.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

class Engine
{
public:
    Engine()              = delete;
    Engine(const Engine&) = delete;
    Engine(Engine&&)      = delete;
    
    inline static void init(const utils::SharedPtr<gfx::Window>& win) {
        s_sharedInstance = utils::UniquePtr<Engine>(new Engine(win));
    }
    inline static Engine& shared() { return *s_sharedInstance; }
    inline static void terminate() { s_sharedInstance.clear(); }

    inline gfx::Window& window() { return *m_window; }

    void runApplication(utils::UniquePtr<Application>&&);

    ~Engine() = default;

private:
    Engine(const utils::SharedPtr<gfx::Window>&);

    static utils::UniquePtr<Engine> s_sharedInstance;

    utils::SharedPtr<gfx::Window> m_window;
    Renderer m_renderer;

    utils::UniquePtr<Application> m_application;
    
public:
    Engine& operator = (const Engine&) = delete;
    Engine& operator = (Engine&&)      = delete;
};

}

#endif // ENGINE_HPP