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

    m_instance = gfx::Instance::newInstance(gfx::Instance::Descriptor{});
    assert(m_instance);

    m_window = std::make_unique<Window>(Window::Descriptor{
        .width = 1280,
        .height = 720,
        .title = ""
    });

    m_window->addEventCallBack(this, [&](Event& event){ onEvent(event); });
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

    m_renderer = std::make_unique<Renderer>(m_device.get(), m_window->surface());
    m_assetManager = std::make_unique<AssetManager>(m_device.get());

    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    switch (m_device->backend())
    {
        case gfx::Backend::vulkan:
            ImGui_ImplGlfw_InitForVulkan(m_window->glfwWindow(), true);
            break;
        default:
            ImGui_ImplGlfw_InitForOther(m_window->glfwWindow(), true);
            break;
    }

    m_device->imguiInit({gfx::PixelFormat::BGRA8Unorm});
}

void Application::run()
{
    m_running = true;
    while (true)
    {
        ::glfwPollEvents();
        if (m_running == false)
            break;

        m_device->imguiNewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        onUpdate();

        ImGui::Render();
        m_renderer->renderFrame(frameGraph());
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

Application::~Application()
{
    m_device->waitIdle();
    ImGui_ImplGlfw_Shutdown();
    m_device->imguiShutdown();
    ImGui::DestroyContext();
    m_window->clearCallbacks(this);
    ::glfwTerminate();
}

}
