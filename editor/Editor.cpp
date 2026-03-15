/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"

#include "Game-Engine/Components.hpp"
#include "UI/MainMenuBar.hpp"
#include "UI/ViewportPanel.hpp"

#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/FrameGraph.hpp>
#include <Game-Engine/FramePassBuilder.hpp>

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

    GE::Entity teapot = m_editedScene.newEntity("teapot");
    teapot.emplace<GE::TransformComponent>().position.z = -3;
    teapot.emplace<GE::MeshComponent>().id = assetManager().registerAsset<GE::Mesh>(RESOURCE_DIR"/teapot.obj");

    GE::Entity camera = m_editedScene.newEntity("camera");
    camera.emplace<GE::TransformComponent>();
    camera.emplace<GE::CameraComponent>();
    m_editedScene.setActiveCamera(camera);

    GE::Entity light = m_editedScene.newEntity("light");
    light.emplace<GE::TransformComponent>();
    light.emplace<GE::LightComponent>();
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
        .textures = {
            { .name = "viewportBackBuffer", .size = m_viewportSize,             .pixelFormat = gfx::PixelFormat::BGRA8Unorm },
            { .name = "depthBuffer",        .size = m_viewportSize,             .pixelFormat = gfx::PixelFormat::Depth32Float },
            { .name = "windowBackBuffer",   .size = window().frameBufferSize(), .pixelFormat = gfx::PixelFormat::BGRA8Unorm },
        },
        .passes = {
            GE::FlatGeometryPassBuilder(&m_editedScene, &assetManager())
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
