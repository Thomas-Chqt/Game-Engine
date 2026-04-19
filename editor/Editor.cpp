/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"

#include "Game-Engine/ICamera.hpp"
#include "Game-Engine/InputFwd.hpp"
#include "UI/ContentBrowserPanel.hpp"
#include "UI/EntityInspectorPanel.hpp"
#include "UI/MainMenuBar.hpp"
#include "UI/ProjectPropertiesPanel.hpp"
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

#include <cassert>
#include <imgui.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <functional>
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
    editorCameraMoveInput.setMapper<GE::KeyboardButton>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
        .xPos = GE::KeyboardButton::d, .xNeg = GE::KeyboardButton::a, .yPos = GE::KeyboardButton::w, .yNeg = GE::KeyboardButton::s,
    });
    GE::Range2DInput editorCameraRotationInput;
    editorCameraRotationInput.setMapper<GE::KeyboardButton>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
        .xPos = GE::KeyboardButton::up, .xNeg = GE::KeyboardButton::down, .yPos = GE::KeyboardButton::right, .yNeg = GE::KeyboardButton::left,
    });

    editorCameraMoveInput.callback = [editorCamera=&m_editorCamera](const glm::vec2& value) { editorCamera->onMoveInput(value); };
    editorCameraRotationInput.callback = [editorCamera=&m_editorCamera](const glm::vec2& value) { editorCamera->onRotationInput(value); };

    m_editorInputContext.addInput(editorCameraMoveInput);
    m_editorInputContext.addInput(editorCameraRotationInput);

    pushInputContext(&m_editorInputContext);
    pushInputContext(&m_imguiInputContext);

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

void Editor::saveEditedScene()
{
    m_project.setScene(m_editedScene.first, m_editedScene.second.makeDescriptor());
}

void Editor::saveProject()
{
    saveEditedScene();
    if (m_projectFilePath.has_parent_path())
        std::filesystem::create_directories(m_projectFilePath.parent_path());

    YAML::Emitter out;
    out << YAML::convert<Project>::encode(m_project);
    if (!out.good())
        throw std::runtime_error("failed to serialize project");

    std::ofstream file(m_projectFilePath);
    if (!file.is_open())
        throw std::runtime_error("failed to open project file for writing");

    file << out.c_str();
    if (!file.good())
        throw std::runtime_error("failed to write project file");
}

void Editor::startGame()
{
    assert(m_game.has_value() == false);
    saveEditedScene();
    m_game.emplace(&assetManager(), m_project.makeGameDescriptor());
    setPrimaryInputContext(m_game->inputContext());
}

void Editor::stopGame()
{
    assert(m_game.has_value());
    setPrimaryInputContext(m_editorInputContext);
    m_game.reset();
}

void Editor::setPrimaryInputContext(GE::InputContext& inputContext)
{
    popInputContext();
    popInputContext();

    pushInputContext(&inputContext);
    pushInputContext(&m_imguiInputContext);
}

void Editor::rebuildFrameGraph()
{
    std::function<GE::Scene*()> getScene = [this]() -> GE::Scene* {
        if (m_game.has_value())
            return &m_game->activeScene();
        return &m_editedScene.second;
    };

    std::function<GE::ICamera*()> getCamera = [this]() -> GE::ICamera*{
        if (m_game.has_value())
            return nullptr;
        return &m_editorCamera;
    };

    m_frameGraph = GE::FrameGraph(GE::FrameGraph::Descriptor{
        .backBufferName = "windowBackBuffer",
        .textures = {
            { .name = "viewportBackBuffer", .size = m_viewportSize,             .pixelFormat = gfx::PixelFormat::BGRA8Unorm },
            { .name = "depthBuffer",        .size = m_viewportSize,             .pixelFormat = gfx::PixelFormat::Depth32Float },
            { .name = "windowBackBuffer",   .size = window().frameBufferSize(), .pixelFormat = gfx::PixelFormat::BGRA8Unorm },
        },
        .passes = {
            GE::FlatGeometryPassBuilder(std::move(getScene), std::move(getCamera))
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
    static bool projectPropertiesOpen = false;

    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport();

    MainMenuBar()
        .on_File_Save(!m_projectFilePath.empty() ? [this]() { saveProject(); } : std::function<void()>())
        .on_Project_Properties([]() { projectPropertiesOpen = true; })
        .on_Project_Stop(m_game.has_value() ? [this]() { stopGame(); } : std::function<void()>())
        .on_Project_Run(!m_game.has_value() ? [this]() { startGame(); } : std::function<void()>())
        .render();

    ViewportPanel(m_viewportSize)
        .onResize([this](const std::pair<uint32_t, uint32_t> & size){
            m_viewportSize = size;
            rebuildFrameGraph();
        })
        .render();

    SceneGraphPanel(&m_editedScene.second, &m_selectedEntity)
        .render();

    EntityInspectorPanel(m_selectedEntity)
        .render();

    ContentBrowserPanel("Scenes", "scene_dnd", sizeof(GE::Scene::Descriptor))
        .render(m_project.scenes() | std::views::transform([](auto& e) { return std::make_pair(e.second.name, (const void*)&e.second); }));

    ProjectPropertiesPanel(&m_project, &m_projectFilePath, &projectPropertiesOpen)
        .render();

    ImGui::Render();
}

} // namespace GE_Editor
