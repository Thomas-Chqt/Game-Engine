/*
 * ---------------------------------------------------
 * ProjectPropertiesPanel.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "UI/ProjectPropertiesPanel.hpp"

#include <imgui.h>

#include <cstring>

namespace GE_Editor
{

ProjectPropertiesPanel::ProjectPropertiesPanel(Project* project, std::filesystem::path* projectFilePath, bool* isOpen)
    : m_project(project)
    , m_projectFilePath(projectFilePath)
    , m_isOpen(isOpen)
{
}

void ProjectPropertiesPanel::render()
{
    if (m_project == nullptr || m_projectFilePath == nullptr || m_isOpen == nullptr || *m_isOpen == false)
        return;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 windowSize(viewport->Size.x * 0.32f, viewport->Size.y * 0.34f);
    ImGui::SetNextWindowPos(
        ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.5f),
        ImGuiCond_Appearing,
        ImVec2(0.5f, 0.5f)
    );
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_Appearing);

    constexpr ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDocking
        | ImGuiWindowFlags_NoCollapse;

    if (ImGui::Begin("Project properties", m_isOpen, flags))
    {
        ImGui::PushItemWidth(-1.0f);
        ImGui::Spacing();

        char projectName[256];
        ImGui::TextUnformatted("Project name");
        std::strncpy(projectName, m_project->name().c_str(), sizeof(projectName));
        projectName[255] = '\0';
        if (ImGui::InputText("##project_name", projectName, sizeof(projectName)))
            m_project->setName(projectName);

        ImGui::Spacing();

        char projectPath[512];
        ImGui::TextUnformatted("Project file");
        std::strncpy(projectPath, m_projectFilePath->string().c_str(), sizeof(projectPath));
        projectPath[511] = '\0';
        if (ImGui::InputText("##project_file", projectPath, sizeof(projectPath)))
            *m_projectFilePath = projectPath;

        ImGui::Spacing();

        const auto& scenes = m_project->scenes();
        const auto currentStartScene = scenes.find(m_project->startScene().first);
        ImGui::TextUnformatted("Start scene");
        const char* previewValue = currentStartScene != scenes.end() ? currentStartScene->second.name.c_str() : "none";
        if (ImGui::BeginCombo("##start_scene", previewValue))
        {
            for (const auto& [sceneId, descriptor] : scenes)
            {
                const bool isSelected = (sceneId == m_project->startScene().first);
                if (ImGui::Selectable(descriptor.name.c_str(), isSelected))
                    m_project->setStartScene(sceneId);
                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Button("Close", ImVec2(90.0f, 0.0f)))
            *m_isOpen = false;

        if (m_projectFilePath->empty())
        {
            ImGui::SameLine();
            ImGui::TextDisabled("File > Save uses this path.");
        }
    }
    ImGui::End();
}

}
