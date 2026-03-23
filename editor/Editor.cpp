/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"

#include "Game-Engine/Scene.hpp"
#include "UI/EntityInspectorPanel.hpp"
#include "UI/MainMenuBar.hpp"
#include "UI/SceneGraphPanel.hpp"
#include "UI/ViewportPanel.hpp"

#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/FrameGraph.hpp>
#include <Game-Engine/FramePassBuilder.hpp>
#include <Game-Engine/Components.hpp>

#include <Graphics/Enums.hpp>

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
    if (event.dispatch<GE::WindowRequestCloseEvent>([&](GE::WindowRequestCloseEvent&) { terminate(); }))
        return;
    if (event.dispatch<GE::WindowResizeEvent>([&](GE::WindowResizeEvent&) { rebuildFrameGraph(); }))
        return;
}

void Editor::rebuildFrameGraph()
{
    m_frameGraph = GE::FrameGraph(GE::FrameGraph::Descriptor{
        .textures = {
            { .name = "viewportBackBuffer", .size = m_viewportSize,             .pixelFormat = gfx::PixelFormat::BGRA8Unorm },
            { .name = "depthBuffer",        .size = m_viewportSize,             .pixelFormat = gfx::PixelFormat::Depth32Float },
            { .name = "windowBackBuffer",   .size = window().frameBufferSize(), .pixelFormat = gfx::PixelFormat::BGRA8Unorm },
        },
        .backBufferName = "windowBackBuffer",
        .passes = {
            GE::FlatGeometryPassBuilder(m_project.editedScene(), &assetManager())
                .setColorAttachment("viewportBackBuffer")
                .setDepthAttachment("depthBuffer"),
            GE::ImguiPassBuilder()
                .setColorAttachment("windowBackBuffer")
                .addSampledTexture("viewportBackBuffer")
        }
    });
}

void Editor::renderImgui()
{
    GE::Scene* editedScene = m_project.editedScene();
    GE::Entity selectedEntity = m_project.selectedEntity();

    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    MainMenuBar()
        .render();

    ViewportPanel(&m_viewportSize)
        .onResize([this](auto){ rebuildFrameGraph(); })
        .render();

    SceneGraphPanel(editedScene, &selectedEntity)
        .render();

    EntityInspectorPanel(selectedEntity)
        .render();

    ImGui::Render();

    m_project.setSelectedEntity(selectedEntity);
}

} // namespace GE_Editor
