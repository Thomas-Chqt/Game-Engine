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

    void run();
    inline void terminate() { m_running = false; }

    virtual ~Application();

protected:
    virtual void onUpdate() = 0;
    virtual void onImGuiRender() = 0;
    virtual void onWindowResizeEvent(gfx::WindowResizeEvent&) = 0;
    virtual void onWindowRequestCloseEvent(gfx::WindowRequestCloseEvent&) = 0;

    inline void pushInputCtx(InputContext* ctx) { m_inputContextStack.append(ctx); }
    inline void popInputCtx() { m_inputContextStack.pop(--m_inputContextStack.end()); }
    inline void setActiveInputCtx(InputContext* ctx) { m_activeInputContext = ctx; }

private:
    void onEvent(gfx::Event&);
    void onInputEvent(gfx::InputEvent&);

protected:
    utils::SharedPtr<gfx::Window> m_window;
    Renderer m_renderer;

private:
    bool m_running = false;

    utils::Array<InputContext*> m_inputContextStack;
    InputContext* m_activeInputContext = nullptr;

public:
    Application& operator = (const Application&) = delete;
    Application& operator = (Application&&)      = delete;
};

}

#endif // APPLICATION_HPP