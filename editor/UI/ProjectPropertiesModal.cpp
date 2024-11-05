/*
 * ---------------------------------------------------
 * ProjectPropertiesModal.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/30 18:14:29
 * ---------------------------------------------------
 */

#include "UI/ProjectPropertiesModal.hpp"
#include "UtilsCPP/String.hpp"
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;


namespace GE
{

ProjectPropertiesModal::ProjectPropertiesModal(bool& isPresented, Project& project)
    : m_isPresented(isPresented), m_project(project)
{
}

void ProjectPropertiesModal::render()
{
    if (m_isPresented)
    {
        ImGui::OpenPopup("Project properties");
        if (s_needBufUpdate)
        {
            m_project.name().SAFECPY(s_nameBuff);
            utils::String(m_project.ressourcesDir().c_str()).SAFECPY(s_ressourceDirBuff);
            utils::String(m_project.scriptLib().c_str()).SAFECPY(s_scriptsLibBuff);
            s_needBufUpdate = false;
        }
    }
    if (ImGui::BeginPopupModal("Project properties", &m_isPresented, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Name##projectName", s_nameBuff, sizeof(s_nameBuff));
        ImGui::InputText("Ressource directory##projectRessourcesDir", s_ressourceDirBuff, sizeof(s_ressourceDirBuff));
        ImGui::InputText("Script library path##projectScriptsLib", s_scriptsLibBuff, sizeof(s_scriptsLibBuff));

        if (ImGui::Button("Cancel"))
            closePopup();

        ImGui::SameLine();
        ImGui::BeginDisabled(!isRessourceDirValid() || !isScriptLibValid());
        if (ImGui::Button("Ok"))
        {
            m_project.setName(s_nameBuff);
            m_project.setRessourcesDir(fs::path(s_ressourceDirBuff));
            m_project.setScriptLib(fs::path(s_scriptsLibBuff));
            closePopup();
        }
        ImGui::EndDisabled();
        ImGui::EndPopup();
    }
}

void ProjectPropertiesModal::closePopup()
{
    s_needBufUpdate = true;
    m_isPresented = false;
    ImGui::CloseCurrentPopup();
}

bool ProjectPropertiesModal::isRessourceDirValid()
{
    const fs::path path = s_ressourceDirBuff;
    return path.empty() || (path.is_absolute() && fs::is_directory(path)) || (
        fs::path(m_project.savePath()).replace_filename(path).is_absolute() &&
        fs::is_directory(fs::path(m_project.savePath()).replace_filename(path))
    );
}

bool ProjectPropertiesModal::isScriptLibValid()
{
    const fs::path path = s_scriptsLibBuff;
    return path.empty() || (path.is_absolute() && fs::is_regular_file(path)) || (
        fs::path(m_project.savePath()).replace_filename(path).is_absolute() &&
        fs::is_regular_file(fs::path(m_project.savePath()).replace_filename(path))
    );
}

}