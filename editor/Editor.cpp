/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"

#include "Game-Engine/InputFwd.hpp"
#include "UI/ContentBrowserPanel.hpp"
#include "UI/EntityInspectorPanel.hpp"
#include "UI/MainMenuBar.hpp"
#include "UI/SceneGraphPanel.hpp"
#include "UI/ViewportPanel.hpp"

#include <Game-Engine/Event.hpp>
#include <Game-Engine/RawInput.hpp>
#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/FrameGraph.hpp>
#include <Game-Engine/FramePassBuilder.hpp>
#include <Game-Engine/Components.hpp>
#include <Game-Engine/Scene.hpp>

#include <Graphics/Enums.hpp>

#include <imgui.h>

#include <memory>
#include <ranges>
#include <utility>

std::unique_ptr<GE::Application> createApplication(int argc, char* argv[])
{
    return std::make_unique<GE_Editor::Editor>();
}

namespace GE_Editor
{

Editor::Editor()
    : m_project{}
    , m_editedScene{m_project.startScene().first, GE::Scene(&assetManager(), m_project.startScene().second)}

{
    GE::Range2DInput editorCameraMoveInput;
    editorCameraMoveInput.callback = [editorCamera=&m_editorCamera](const glm::vec2& value) { editorCamera->onMoveInput(value); };
    editorCameraMoveInput.setMapper<GE::KeyboardButton>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
        .xPos = GE::KeyboardButton::d,
        .xNeg = GE::KeyboardButton::a,
        .yPos = GE::KeyboardButton::w,
        .yNeg = GE::KeyboardButton::s,
    });

    GE::Range2DInput editorCameraRotationInput;
    editorCameraRotationInput.callback = [editorCamera=&m_editorCamera](const glm::vec2& value) { editorCamera->onRotationInput(value); };
    editorCameraRotationInput.setMapper<GE::KeyboardButton>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
        .xPos = GE::KeyboardButton::up,
        .xNeg = GE::KeyboardButton::down,
        .yPos = GE::KeyboardButton::right,
        .yNeg = GE::KeyboardButton::left,
    });

    m_editorInputContext.addInput(editorCameraMoveInput);
    m_editorInputContext.addInput(editorCameraRotationInput);

    pushInputContext(&m_editorInputContext);
    pushInputContext(&m_imguiInputContext);

    m_viewport.setCamera(&m_editorCamera);
    rebuildFrameGraph();
}

void Editor::onUpdate()
{
    renderImgui();
}

void Editor::onEvent(GE::Event& event)
{
    if (event.dispatch<GE::WindowRequestCloseEvent>([&](auto&) { terminate(); })) return;
    if (event.dispatch<GE::WindowResizeEvent>([&](auto&) { rebuildFrameGraph(); })) return;
}

void Editor::rebuildFrameGraph()
{
    m_frameGraph = GE::FrameGraph(GE::FrameGraph::Descriptor{
        .backBufferName = "windowBackBuffer",
        .textures = {
            { .name = "viewportBackBuffer", .size = m_viewport.size(),          .pixelFormat = gfx::PixelFormat::BGRA8Unorm },
            { .name = "depthBuffer",        .size = m_viewport.size(),          .pixelFormat = gfx::PixelFormat::Depth32Float },
            { .name = "windowBackBuffer",   .size = window().frameBufferSize(), .pixelFormat = gfx::PixelFormat::BGRA8Unorm },
        },
        .passes = {
            GE::FlatGeometryPassBuilder(&m_editedScene.second, m_viewport.camera())
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

    ViewportPanel(m_viewport.size())
        .onResize([this](const std::pair<uint32_t, uint32_t> & size){
            m_viewport.setSize(size);
            rebuildFrameGraph();
        })
        .render();

    SceneGraphPanel(&m_editedScene.second, &m_selectedEntity)
        .render();

    EntityInspectorPanel(m_selectedEntity)
        .render();

    ContentBrowserPanel("Scenes", "scene_dnd", sizeof(GE::Scene::Descriptor))
        .render(m_project.scenes() | std::views::transform([](auto& e) { return std::make_pair(e.second.name, (const void*)&e.second); }));

    ImGui::Render();
}

} // namespace GE_Editor
