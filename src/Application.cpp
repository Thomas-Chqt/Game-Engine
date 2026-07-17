/*
 * ---------------------------------------------------
 * Application.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/02 17:15:13
 * ---------------------------------------------------
 */

#include "Game-Engine/Application.hpp"
#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Event.hpp"
#include "Game-Engine/Window.hpp"
#include "Game-Engine/Renderer.hpp"

#include <Graphics/Instance.hpp>
#include <Graphics/Surface.hpp>
#include <Graphics/Device.hpp>

#include <GLFW/glfw3.h>
#include <gfx_glfw/gfx_glfw.hpp>

#include <ranges>
#include <tracy/Tracy.hpp>
#include <tracy/TracyC.h>

#include <algorithm>
#include <cassert>
#include <memory>
#include <thread>
#include <utility>

namespace GE
{

Application::Application()
    : m_threadPool(std::max<std::size_t>(1, std::thread::hardware_concurrency()))
{
    [[maybe_unused]] auto res = ::glfwInit();
    assert(res == GLFW_TRUE);
    m_glfwGuard = { std::bit_cast<void*>(1zu), [](void*){ ::glfwTerminate(); } };

    m_instance = gfx::Instance::newInstance(gfx::Instance::Descriptor{
        .instanceExtension = gfx::glfw::getInstanceExtension()
    });
    assert(m_instance);

    m_window = std::make_unique<Window>(Window::Descriptor{
        .width = 1280,
        .height = 720,
        .title = ""
    });

    m_window->addEventCallBack(this, [&](Event& event)
    {
        event.dispatch<InputEvent>([this](InputEvent& inputEvent)
        {
            for (auto & it : std::ranges::reverse_view(m_inputContextStack))
                it->onInputEvent(inputEvent);
        });
        if (event.processed() == false)
            onEvent(event);
    });
    m_window->createSurface(m_instance.get());

    m_device = m_instance->newDevice(gfx::Device::Descriptor{
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

    m_assetManager = std::make_unique<AssetManager>(m_device.get(), &m_threadPool);
    m_renderer = std::make_unique<Renderer>(m_device.get(), m_assetManager.get(), m_window->surface());
}

void Application::run()
{
    m_running = true;
    while (true)
    {
        TracyCZoneN(glfwPollEventsCtx, "Application::poll events", true);
        ::glfwPollEvents();
        TracyCZoneEnd(glfwPollEventsCtx);

        TracyCZoneN(applicationDispachInputsCtx, "Application::dispatch input", true);
        for (InputContext* inputContext : m_inputContextStack)
            inputContext->dispatchInputs();
        TracyCZoneEnd(applicationDispachInputsCtx);

        if (m_running == false)
            break;

        onUpdate();

        FrameGraphBuilder frameGraphBuilder = m_renderer->newFrameGraphBuilder();
        recordFrameGraph(frameGraphBuilder);
        FrameGraph frameGraph = std::move(frameGraphBuilder).build();
        m_renderer->renderFrame(std::move(frameGraph));

        FrameMark;
    }
}

void Application::pushInputContext(InputContext* inputContext)
{
    assert(inputContext != nullptr);
    m_inputContextStack.push_back(inputContext);
}

void Application::popInputContext()
{
    assert(m_inputContextStack.size() > 0);
    m_inputContextStack.pop_back();
}

}
