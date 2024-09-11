/*
 * ---------------------------------------------------
 * Application.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/02 16:56:11
 * ---------------------------------------------------
 */

#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "Graphics/Window.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

class Application
{
public:
    Application();
    Application(const Application&) = delete;
    Application(Application&&)      = delete;

    void run();
    inline void terminate() { m_running = false; }

    virtual ~Application();

protected:
    virtual void onUpdate() = 0;
    virtual void onImGuiRender() = 0;
    virtual void onEvent(gfx::Event&) = 0;

    utils::SharedPtr<gfx::Window> m_window;
    Renderer m_renderer;

private:
    bool m_running = false;

public:
    Application& operator = (const Application&) = delete;
    Application& operator = (Application&&)      = delete;
};

}

#endif // APPLICATION_HPP