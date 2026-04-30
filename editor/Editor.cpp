/*
 * ---------------------------------------------------
 * Editor.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/03 14:42:56
 * ---------------------------------------------------
 */

#include "Editor.hpp"

#include "Project.hpp"
#include "UI/EntityInspectorPanel.hpp"
#include "UI/Group.hpp"
#include "UI/MainMenuBar.hpp"
#include "UI/Pane.hpp"
#include "UI/ProjectPropertiesPanel.hpp"
#include "UI/RessourceBrowserPanel.hpp"
#include "UI/SceneGraphPanel.hpp"
#include "UI/TileGrid.hpp"
#include "UI/ViewportPanel.hpp"

#include <Game-Engine/Event.hpp>
#include <Game-Engine/RawInput.hpp>
#include <Game-Engine/AssetManager.hpp>
#include <Game-Engine/ECSView.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/FrameGraph.hpp>
#include <Game-Engine/FramePassBuilder.hpp>
#include <Game-Engine/Components.hpp>
#include <Game-Engine/Script.hpp>
#include <Game-Engine/Scene.hpp>
#include <Game-Engine/ICamera.hpp>
#include <Game-Engine/InputFwd.hpp>
#include <Game-Engine/InputContext.hpp>

#include <Graphics/Enums.hpp>

#include <format>
#include <imgui.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <ranges>
#include <functional>
#include <stdexcept>
#include <utility>
#include <cassert>

std::unique_ptr<GE::Application> createApplication(int argc, char* argv[])
{
    return std::make_unique<GE_Editor::Editor>(argc, argv);
}

namespace GE_Editor
{

namespace
{

Project loadProjectFile(const std::filesystem::path& path)
{
    Project project;

    std::ifstream file(path);
    if (file.is_open() == false)
        throw std::runtime_error(std::format("unable to open file : {}", path.string()));

    YAML::Node projectNode = YAML::Load(file);

    if (YAML::convert<Project>::decode(projectNode, project) == false)
        throw std::runtime_error(std::format("unable to load project file : {}", path.string()));

    return project;
}

void makeEditorInputs(GE::InputContext& context)
{
    GE::Range2DInput editorCameraMoveInput;
    editorCameraMoveInput.setMapper<GE::KeyboardButton>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
        .xPos = GE::KeyboardButton::d, .xNeg = GE::KeyboardButton::a, .yPos = GE::KeyboardButton::w, .yNeg = GE::KeyboardButton::s,
    });
    context.addInput("camera_move", editorCameraMoveInput);

    GE::Range2DInput editorCameraRotationInput;
    editorCameraRotationInput.setMapper<GE::KeyboardButton>(GE::InputMapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor{
        .xPos = GE::KeyboardButton::up, .xNeg = GE::KeyboardButton::down, .yPos = GE::KeyboardButton::right, .yNeg = GE::KeyboardButton::left,
    });
    context.addInput("camera_rotate", editorCameraRotationInput);
}

}

Editor::Editor(int argc, char* argv[])
    : m_projectFilePath(argc == 2 ? std::filesystem::path(argv[1]) : std::filesystem::path())
    , m_project(m_projectFilePath.empty() ? Project() : loadProjectFile(m_projectFilePath))
    , m_editedScene{m_project.startScene().first, GE::Scene(&assetManager(), m_project.startScene().second)}
{
    makeEditorInputs(m_editorInputContext);

    m_editorInputContext.setInputCallback<GE::Range2DInput>("camera_move", [editorCamera=&m_editorCamera](const glm::vec2& value) { editorCamera->onMoveInput(value); }) ;
    m_editorInputContext.setInputCallback<GE::Range2DInput>("camera_rotate", [editorCamera=&m_editorCamera](const glm::vec2& value) { editorCamera->onRotationInput(value); });

    pushInputContext(&m_editorInputContext);
    pushInputContext(&m_imguiInputContext);

    rebuildFrameGraph();

    ImGui::LoadIniSettingsFromMemory(m_project.imguiSettings().c_str());

    if (std::filesystem::exists(m_project.scriptLib()))
        reloadScriptLib();
}

void Editor::onUpdate()
{
    processDropedFiles();

    if (m_game.has_value())
    {
        // game update
        // TODO ? maybe move into game class
        for (auto [scriptComponent] : m_game->activeScene().ecsWorld() | GE::ECSView<GE::ScriptComponent>())
        {
            assert(scriptComponent.instance);
            scriptComponent.instance->onUpdate();
        }
    }

    renderImgui();
}

void Editor::onEvent(GE::Event& event)
{
    if (event.dispatch<GE::WindowRequestCloseEvent>([&](auto&) { terminate(); })) return;
    if (event.dispatch<GE::WindowResizeEvent>([&](auto&) { rebuildFrameGraph(); })) return;
}

void Editor::loadProject(const std::filesystem::path& path)
{
    m_projectFilePath = path; // if we allow file loading error (not terminating the program on load error)
                              // this will need to go after the yaml parsing
    m_project = loadProjectFile(path);

    m_editedScene = {
        m_project.startScene().first,
        GE::Scene(&assetManager(), m_project.startScene().second)
    };

    ImGui::LoadIniSettingsFromMemory(m_project.imguiSettings().c_str());

    if (std::filesystem::exists(m_project.scriptLib()))
        reloadScriptLib();

    m_selectedEntity = {};
    m_editorCamera = {};
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

void Editor::reloadScriptLib()
{
    assert(m_game.has_value() == false);
    assert(std::ranges::all_of(
        m_editedScene.second.ecsWorld() | GE::ECSView<GE::ScriptComponent>(),
        [](const auto& e) -> bool { return e.template get<0>().instance == nullptr; }
    ));
    assert(std::filesystem::exists(m_project.scriptLib()));

    if (m_project.scriptLib().empty())
    {
        m_listScriptNames = {};
        m_listScriptParameters = {};
        m_makeScriptInstance = {};
        return;
    }

    GE::ScriptLibraryManager manager(m_project.scriptLib());
    m_listScriptNames = manager.listScriptNamesFunction();
    m_listScriptParameters = manager.listScriptParametersFunction();
    m_makeScriptInstance = manager.makeScriptInstanceFunction();
}

void Editor::startGame()
{
    assert(m_game.has_value() == false);
    saveEditedScene();
    m_game.emplace(&assetManager(), m_makeScriptInstance, m_listScriptParameters, m_project.makeGameDescriptor());
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

void Editor::processDropedFiles()
{
    while (const std::optional<std::filesystem::path> droppedFile = window().popDroppedFile())
    {
        if (std::filesystem::is_regular_file(*droppedFile) && droppedFile->extension() == ".geproj")
            loadProject(*droppedFile);
    }
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
        .on_Project_ReloadScriptLib(!m_game.has_value() ? [this]() { reloadScriptLib(); } : std::function<void()>())
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

    EntityInspectorPanel(m_selectedEntity, &m_editedScene.second, m_listScriptNames, m_listScriptParameters)
        .render();

    constexpr float TILE_SIZE = 60.0f;

    UI::Pane("Scenes",
        UI::TileGrid(m_project.scenes() | std::views::transform([](const auto& scene) {
            return UI::Group(
                UI::Button(scene.second.name)
                    .size(TILE_SIZE, TILE_SIZE)
                    .dragDropSource([name=scene.second.name](){
                        ImGui::SetDragDropPayload("scene_dnd", name.c_str(), name.size());
                        ImGui::Text("%s", name.c_str());
                    }),
                UI::Text(scene.second.name)
                    .size(TILE_SIZE, ImGui::GetTextLineHeightWithSpacing())
            );
        }))
    )
    .render();

    ProjectPropertiesPanel(&m_project, &m_projectFilePath, &projectPropertiesOpen).render();

    if (std::filesystem::exists(m_project.resourceDir()))
        UI::RessourceBrowserPanel(m_project.resourceDir()).render();

    ImGui::Render();

    if (ImGui::GetIO().WantSaveIniSettings)
    {
        m_project.setImguiSettings(ImGui::SaveIniSettingsToMemory());
        ImGui::GetIO().WantSaveIniSettings = false;
    }
}

} // namespace GE_Editor
