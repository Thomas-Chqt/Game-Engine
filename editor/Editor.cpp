/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"
#include "Graphics/Enums.hpp"
#include "UI/MainMenuBar.hpp"
#include "UI/ViewportPanel.hpp"

#include <Game-Engine/FrameGraph.hpp>
#include <Game-Engine/FramePassBuilder.hpp>

#include <imgui.h>

#include <memory>

std::unique_ptr<GE::Application> createApplication(int argc, char* argv[])
{
    return std::make_unique<GE_Editor::Editor>();
}

namespace GE_Editor
{

Editor::Editor()
{
    rebuildFrameGraph();
}

void Editor::onUpdate()
{
    renderImgui();
}

void Editor::onEvent(GE::Event& event)
{
    if (event.dispatch<GE::WindowRequestCloseEvent>([&](GE::WindowRequestCloseEvent&) {
            terminate();
        }))
        return;
    if (event.dispatch<GE::WindowResizeEvent>([&](GE::WindowResizeEvent&) {
            rebuildFrameGraph();
        }))
        return;
}

void Editor::rebuildFrameGraph()
{
    m_frameGraph = GE::FrameGraph(GE::FrameGraph::Descriptor{
        .backBufferName = "windowBackBuffer",
        .passes = {
            GE::ClearPassBuilder()
                .setRenderSize(m_viewportSize)
                .setColorAttachment("viewportBackBuffer")
                .setClearColor({0.0f, 1.0f, 1.0f})
                .setDepthAttachment("depthBuffer")
                .setClearDepth(1.0f),
            GE::ImguiPassBuilder()
                .setRenderSize(window().frameBufferSize())
                .setColorAttachment("windowBackBuffer", gfx::PixelFormat::BGRA8Unorm, gfx::LoadAction::clear)
                .addSampledAttachment("viewportBackBuffer")
        }
    });
}

void Editor::renderImgui()
{
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    MainMenuBar()
        .render();

    ViewportPanel(&m_viewportSize)
        .onResize([this](auto){ rebuildFrameGraph(); })
        .render();

    ImGui::Render();
}

} // namespace GE_Editor
