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

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Event.hpp"
#include "Game-Engine/Window.hpp"
#include "Game-Engine/Renderer.hpp"

#include <Graphics/Device.hpp>

#include <cstddef>
#include <memory>

namespace GE
{

class Application
{
public:
    Application();
    Application(const Application&) = delete;
    Application(Application&&)      = delete;

    inline Window& window() { return *m_window; }

    void run();
    inline void terminate() { m_running = false; }

    virtual void onUpdate() = 0;
    virtual void onEvent(Event&) = 0;

    virtual ~Application();

private:
    std::unique_ptr<Window> m_window = nullptr;
    std::unique_ptr<gfx::Device> m_device = nullptr;
    std::unique_ptr<Renderer> m_renderer = nullptr;
    std::unique_ptr<AssetManager> m_assetManager = nullptr;

    bool m_running = false;

public:
    Application& operator = (const Application&) = delete;
    Application& operator = (Application&&)      = delete;
};

}

#endif // APPLICATION_HPP
