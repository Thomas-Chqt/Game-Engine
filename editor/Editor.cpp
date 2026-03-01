/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"

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
    ImGui::ShowDemoWindow();
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
        .backBufferName = "backBuffer",
        .passes = {
            GE::ClearPassBuilder()
                .setRenderSize(window().frameBufferSize())
                .setColorAttachment("backBuffer")
                .setClearColor({0.0f, 0.0f, 0.0f})
                .setDepthAttachment("depthBuffer")
                .setClearDepth(1.0f),
            GE::ImguiPassBuilder()
                .setRenderSize(window().frameBufferSize())
                .setColorAttachment("backBuffer")
        }
    });
}

} // namespace GE_Editor
