/*
 * ---------------------------------------------------
 * Editor.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/02 16:04:23
 * ---------------------------------------------------
 */

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "ImGuiInputContext.hpp"
#include "Project.hpp"

#include <Game-Engine/Application.hpp>
#include <Game-Engine/FrameGraph.hpp>
#include <Game-Engine/InputContext.hpp>
#include <Game-Engine/Input.hpp>
#include <Game-Engine/ScriptLibrary.hpp>
#include <Game-Engine/Scene.hpp>
#include <Game-Engine/Entity.hpp>

#include <future>
#include <imgui.h>

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace GE_Editor
{

class Editor : public GE::Application
{
public:
    Editor() = delete;
    Editor(const Editor&) = delete;
    Editor(Editor&&) = delete;

    Editor(std::span<const char*> args);

    void onUpdate() override;
    void onEvent(GE::Event& event) override;

    void recordFrameGraph(GE::FrameGraphBuilder&) override;

    void loadProject(Project&&);
    void saveProject();

    void editScene(std::optional<std::string_view> name);
    void reloadScriptLib();

    void startGame();
    void stopGame();

    ~Editor() override = default;

private:
    void setPrimaryInputContext(GE::InputContext&);
    void processDropedFiles();
    void renderImgui();
    void syncEditedScene();

    GE::InputContext m_editorInputContext;
    ImGuiInputContext m_imguiInputContext;

    std::optional<std::filesystem::path> m_projectFilePath;

    std::string m_projectName;

    std::optional<std::filesystem::path> m_resourceDir;
    std::optional<std::filesystem::path> m_scriptLibPath;

    std::vector<std::pair<std::string, GE::VInput>> m_gameInputs;

    std::vector<GE::Scene::Descriptor> m_sceneDescriptors;
    std::string m_startSceneName;

    EditorCamera m_editorCamera;
    std::optional<GE::Scene> m_editedScene;
    std::optional<GE::Entity> m_selectedEntity;

    std::optional<GE::Game> m_game;

    std::pair<uint32_t, uint32_t> m_viewportSize = {0, 0};
    std::optional<std::pair<uint32_t, uint32_t>> m_viewportReadbackRequest;
    std::future<GE::EntityID> m_viewportReadbackFuture;

    std::optional<GE::ScriptLibrary> m_scriptLibrary;

public:
    Editor& operator=(const Editor&) = delete;
    Editor& operator=(Editor&&) = delete;
};

} // namespace GE

#endif // EDITOR_HPP
