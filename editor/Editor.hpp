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

#include "EditorCamera.hpp"
#include "ImGuiInputContext.hpp"
#include "Project.hpp"

#include <Game-Engine/Application.hpp>
#include <Game-Engine/FrameGraph.hpp>
#include <Game-Engine/InputContext.hpp>
#include <Game-Engine/Input.hpp>
#include <Game-Engine/ScriptLibraryManager.hpp>
#include <Game-Engine/Scene.hpp>

#include <imgui.h>

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>

namespace GE_Editor
{

class Editor : public GE::Application
{
public:
    Editor();
    Editor(const Editor&) = delete;
    Editor(Editor&&) = delete;

    void onUpdate() override;
    void onEvent(GE::Event& event) override;

    inline const GE::FrameGraph& frameGraph() override { return m_frameGraph; }

    ~Editor() override = default;

private:
    void loadProject(const std::filesystem::path&);
    void saveEditedScene();
    void saveProject();
    void reloadScriptLib();
    void startGame();
    void stopGame();

    void setPrimaryInputContext(GE::InputContext&);
    void processDropedFiles();

    void rebuildFrameGraph();
    void renderImgui();

    GE::ScriptLibraryFunctions m_scriptLibraryFunctions;

    std::filesystem::path m_projectFilePath;
    Project m_project;

    std::pair<uint32_t, GE::Scene> m_editedScene;
    GE::Entity m_selectedEntity;

    EditorCamera m_editorCamera;

    GE::InputContext m_editorInputContext;
    ImGuiInputContext m_imguiInputContext;

    std::optional<GE::Game> m_game;

    std::pair<uint32_t, uint32_t> m_viewportSize = {0, 0};
    GE::FrameGraph m_frameGraph; // TODO move into render (something like render->setGraph())

public:
    Editor& operator=(const Editor&) = delete;
    Editor& operator=(Editor&&) = delete;
};

} // namespace GE

#endif // EDITOR_HPP
