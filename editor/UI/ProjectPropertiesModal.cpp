/*
 * ---------------------------------------------------
 * ProjectPropertiesModal.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/30 18:14:29
 * ---------------------------------------------------
 */

#include "UI/ProjectPropertiesModal.hpp"
#include "UI/FileOpenDialog.hpp"
#include "UtilsCPP/String.hpp"
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;


namespace GE
{

ProjectPropertiesModal::ProjectPropertiesModal(bool& isPresented, Project& project, const std::filesystem::path& projSavePath)
    : m_isPresented(isPresented), m_project(project), m_projSavePath(projSavePath)
{
}

void ProjectPropertiesModal::render()
{
    static bool isScriptLibSelectDialogPresented = false;

    if (m_isPresented)
    {
        ImGui::OpenPopup("Project properties");
        if (s_needBufUpdate)
        {
            m_project.name().SAFECPY(s_nameBuff);
            utils::String(m_project.scriptLib().c_str()).SAFECPY(s_scriptsLibBuff);
            s_needBufUpdate = false;
        }
    }
    if (ImGui::BeginPopupModal("Project properties", &m_isPresented, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::BeginDisabled(isScriptLibSelectDialogPresented);
            
        ImGui::InputText("Name##projectName", s_nameBuff, sizeof(s_nameBuff));
        ImGui::InputText("Script library path##projectScriptsLib", s_scriptsLibBuff, sizeof(s_scriptsLibBuff));
        ImGui::SameLine();
        if (ImGui::Button("..."))
            isScriptLibSelectDialogPresented = true;

        if (ImGui::Button("Cancel"))
        {
            closePopup();
            if (m_onCancel)
                m_onCancel();
        }

        ImGui::SameLine();
        ImGui::BeginDisabled(!isScriptLibValid());
        if (ImGui::Button("Ok"))
        {
            m_project.setName(s_nameBuff);
            m_project.setScriptLib(fs::path(s_scriptsLibBuff));
            closePopup();
            if (m_onOk)
                m_onOk();
        }
        ImGui::EndDisabled();
        ImGui::EndDisabled();

        ImGui::EndPopup();

        FileOpenDialog("Select script lib", isScriptLibSelectDialogPresented)
            .onSelection([&](const fs::path& path) {
                fs::path relativePath = fs::relative(path, fs::path(m_projSavePath).remove_filename());
                utils::String(relativePath.c_str()).SAFECPY(s_scriptsLibBuff);
            })
            .render();
    }
}

void ProjectPropertiesModal::closePopup()
{
    s_needBufUpdate = true;
    m_isPresented = false;
    ImGui::CloseCurrentPopup();
}


bool ProjectPropertiesModal::isScriptLibValid()
{
    fs::path path = s_scriptsLibBuff;
    if (path.empty())
        return true;
    path = fs::path(m_projSavePath).remove_filename() / path;
    return path.is_absolute() && fs::is_regular_file(path);
}

}