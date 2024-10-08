/*
 * ---------------------------------------------------
 * NewProjectPopupModal.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/08 09:27:30
 * ---------------------------------------------------
 */

#include "NewProjectPopupModal.hpp"
#include "imgui.h"
#include <cstring>
#include "TFD/tinyfiledialogs.h"

namespace GE
{

NewProjectPopupModal::NewProjectPopupModal(bool& isPresented, utils::String& projectName, utils::String& path)
    : m_isPresented(isPresented), m_projectName(projectName), m_path(path)
{
}

void NewProjectPopupModal::render()
{
    if (m_isPresented)
        ImGui::OpenPopup("New project");
    if (ImGui::BeginPopupModal("New project", &m_isPresented, ImGuiWindowFlags_AlwaysAutoResize))
    {
        char buff[32];
        std::strncpy(buff, m_projectName, sizeof(buff));
        ImGui::InputText("Name", buff, sizeof(buff));
        m_projectName = utils::String(buff);

        char buff2[2048];
        std::strncpy(buff2, m_path, sizeof(buff2));
        ImGui::InputText("Dir", buff2, sizeof(buff2));
        m_path = utils::String(buff2);

        ImGui::SameLine();

        if (ImGui::Button("..."))
            m_path = tinyfd_selectFolderDialog("Select a saving destination", nullptr);

        if (ImGui::Button("Create") && m_onCreate)
            m_onCreate();

        ImGui::EndPopup();
    }
    if (m_isPresented == false && m_onClose)
        m_onClose();
}

}