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
#include <imgui.h>
#include <imgui_impl_glfw.h>

#include <cassert>
#include <memory>

namespace GE
{

Application::Application()
{
    auto res = ::glfwInit();
    assert(res == GLFW_TRUE);
    (void)res;
    m_glfwGuard = { (void*)(1), [](void*){ ::glfwTerminate(); } };

    m_instance = gfx::Instance::newInstance(gfx::Instance::Descriptor{});
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
            for (auto it = m_inputContextStack.rbegin(); it != m_inputContextStack.rend(); it++)
            {
                (*it)->onInputEvent(inputEvent);
                if (inputEvent.processed())
                    break;
            }
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

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    switch (m_device->backend())
    {
        case gfx::Backend::vulkan:
            ImGui_ImplGlfw_InitForVulkan(m_window->glfwWindow(), false);
            break;
        default:
            ImGui_ImplGlfw_InitForOther(m_window->glfwWindow(), false);
            break;
    }
    m_device->imguiInit({gfx::PixelFormat::BGRA8Unorm});
    m_imguiGuard = {
        (void*)(1),
        [device=m_device.get()](void*){
            device->imguiShutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
    };

    m_renderer = std::make_unique<Renderer>(m_device.get(), m_window->surface());
    m_assetManager = std::make_unique<AssetManager>(m_device.get());
}

void Application::run()
{
    m_running = true;
    while (true)
    {
        ::glfwPollEvents();

        for (InputContext* inputContext : m_inputContextStack)
            inputContext->dispatchInputs();

        if (m_running == false)
            break;

        m_device->imguiNewFrame();
        ImGui_ImplGlfw_NewFrame();

        onUpdate();

        m_renderer->renderFrame(frameGraph());
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
