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

#include "Project.hpp"

#include <Game-Engine/Application.hpp>
#include <Game-Engine/FrameGraph.hpp>
#include <Game-Engine/Scene.hpp>

#include <imgui.h>

#include <utility>
#include <cstdint>

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

private:
    void rebuildFrameGraph();
    void renderImgui();

    std::filesystem::path m_projectFilePath;
    Project m_project;

    std::pair<uint32_t, GE::Scene> m_editedScene;
    GE::Entity m_selectedEntity;

    std::pair<uint32_t, uint32_t> m_viewportSize = {0, 0};

    GE::FrameGraph m_frameGraph;

public:
    Editor& operator=(const Editor&) = delete;
    Editor& operator=(Editor&&) = delete;
};

} // namespace GE

#endif // EDITOR_HPP
