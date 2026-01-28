/*
 * ---------------------------------------------------
 * Application.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/02 17:15:13
 * ---------------------------------------------------
 */

#include "Game-Engine/Application.hpp"
#include "Game-Engine/Event.hpp"
#include "Game-Engine/Window.hpp"
#include "Game-Engine/Renderer.hpp"

#include <Graphics/Instance.hpp>
#include <Graphics/Surface.hpp>
#include <Graphics/Device.hpp>

#include <cassert>
#include <memory>

namespace GE
{

Application::Application()
{
    auto res = ::glfwInit();
    assert(res == GLFW_TRUE);
    (void)res;

    std::unique_ptr<gfx::Instance> instance = gfx::Instance::newInstance(gfx::Instance::Descriptor{});
    assert(instance);

    m_window = std::make_unique<Window>(Window::Descriptor{
        .width = 1280,
        .height = 720,
        .title = ""
    });

    m_window->addEventCallBack(this, [&](Event& event){ onEvent(event); });
    m_window->createSurface(instance.get());

    m_device = instance->newDevice(gfx::Device::Descriptor{
        .queueCaps = {
            .graphics = true,
            .compute = true,
            .transfer = true,
            .present = {m_window->surface()}}});
    assert(m_device);

    if (m_window->surface()->supportedPixelFormats(*m_device).contains(gfx::PixelFormat::BGRA8Unorm) == false)
        throw std::runtime_error("surface does not support the BGRA8Unorm pixel format");

    if (m_window->surface()->supportedPresentModes(*m_device).contains(gfx::PresentMode::fifo) == false)
        throw std::runtime_error("surface does not support the fifo present mode");

    m_renderer = std::make_unique<Renderer>(m_device.get());
}

void Application::run()
{
    m_running = true;
    while (m_running)
    {
        ::glfwPollEvents();
        onUpdate();
    }
}

Application::~Application()
{
    m_window->clearCallbacks(this);
    ::glfwTerminate();
}

}
