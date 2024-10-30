/*
 * ---------------------------------------------------
 * MainMenuBar.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/30 16:36:33
 * ---------------------------------------------------
 */

#include "UI/MainMenuBar.hpp"
#include <imgui.h>

namespace GE
{

void MainMenuBar::render()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::BeginDisabled(!m_file.neew);
            if (ImGui::MenuItem("New"))
                m_file.neew();
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!m_file.open);
            if (ImGui::MenuItem("Open"))
                m_file.open();
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!m_file.save);
            if (ImGui::MenuItem("Save"))
                m_file.save();
            ImGui::EndDisabled();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Project"))
        {
            ImGui::BeginDisabled(!m_project.properties);
            if (ImGui::MenuItem("Properties"))
                m_project.properties();
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!m_project.scenes);
            if (ImGui::MenuItem("Scene"))
                m_project.scenes();
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!m_project.run);
            if (ImGui::MenuItem("Run"))
                m_project.run();
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!m_project.stop);
            if (ImGui::MenuItem("Stop"))
                m_project.stop();
            ImGui::EndDisabled();

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::BeginDisabled(!m_debug.showDemoWindow);
            if (ImGui::MenuItem("Show demo window"))
                m_debug.showDemoWindow();
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!m_debug.showMetricsWindow);
            if (ImGui::MenuItem("Show metrics window"))
                m_debug.showMetricsWindow();
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!m_debug.startEditedScene);
            if (ImGui::MenuItem("Start edited scene"))
                m_debug.startEditedScene();
            ImGui::EndDisabled();

            ImGui::BeginDisabled(!m_debug.stopEditedScene);
            if (ImGui::MenuItem("Stop edited scene"))
                m_debug.stopEditedScene();
            ImGui::EndDisabled();

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

}
