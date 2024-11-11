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

#include "Graphics/Event.hpp"
#include "Graphics/Window.hpp"
#include "InputManager/InputContext.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/Array.hpp"
#include "UtilsCPP/SharedPtr.hpp"

namespace GE
{

class Application
{
public:
    Application();
    Application(const Application&) = delete;
    Application(Application&&)      = delete;

    inline gfx::Window& window() { return *m_window; }
    inline Renderer& renderer() { return m_renderer; }

    void run();

    virtual void onUpdate() = 0;
    virtual void onImGuiRender() = 0;
    void onEvent(gfx::Event&);
    virtual void onWindowResizeEvent(gfx::WindowResizeEvent&) = 0;
    virtual void onWindowRequestCloseEvent(gfx::WindowRequestCloseEvent&) = 0;

    inline void pushInputCtx(InputContext* ctx) { m_inputContextStack.append(ctx); }
    inline void popInputCtx() { m_inputContextStack.pop(--m_inputContextStack.end()); }
    inline void setDispatchedInputCtx(InputContext* ctx) { m_dispatchedInputContext = ctx; }

    inline void terminate() { m_running = false; }

    virtual ~Application();

private:
    utils::SharedPtr<gfx::Window> m_window;
    Renderer m_renderer;
    utils::Array<InputContext*> m_inputContextStack;
    InputContext* m_dispatchedInputContext = nullptr;

    bool m_running = false;

public:
    Application& operator = (const Application&) = delete;
    Application& operator = (Application&&)      = delete;
};

}

#endif // APPLICATION_HPP