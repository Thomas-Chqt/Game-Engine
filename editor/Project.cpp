/*
 * ---------------------------------------------------
 * Project.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/11 12:07:15
 * ---------------------------------------------------
 */

#include "Project.hpp"
#include "Scene.hpp"
#include "UtilsCPP/String.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

#define DEFAULT_IMGUI_INI\
    "[Window][WindowOverViewport_11111111]\n"\
    "Pos=0,19\n"\
    "Size=1280,701\n"\
    "Collapsed=0\n"\
    "\n"\
    "[Window][Debug##Default]\n"\
    "Pos=60,60\n"\
    "Size=400,400\n"\
    "Collapsed=0\n"\
    "\n"\
    "[Window][Entity inspector]\n"\
    "Pos=999,494\n"\
    "Size=281,226\n"\
    "Collapsed=0\n"\
    "DockId=0x00000004,0\n"\
    "\n"\
    "[Window][Scene graph]\n"\
    "Pos=999,19\n"\
    "Size=281,473\n"\
    "Collapsed=0\n"\
    "DockId=0x00000003,0\n"\
    "\n"\
    "[Window][viewport]\n"\
    "Pos=0,19\n"\
    "Size=997,701\n"\
    "Collapsed=0\n"\
    "DockId=0x00000001,0\n"\
    "\n"\
    "[Window][Project properties]\n"\
    "Pos=423,248\n"\
    "Size=337,287\n"\
    "Collapsed=0\n"\
    "\n"\
    "[Docking][Data]\n"\
    "DockSpace ID=0x7C6B3D9B Window=0xA87D555D Pos=942,394 Size=1280,701 Split=X Selected=0x0BA3B4F3\n"\
    "DockNode  ID=0x00000001 Parent=0x7C6B3D9B SizeRef=1185,701 CentralNode=1 Selected=0x0BA3B4F3\n"\
    "DockNode  ID=0x00000002 Parent=0x7C6B3D9B SizeRef=281,701 Split=Y Selected=0xF5BE1C77\n"\
    "DockNode  ID=0x00000003 Parent=0x00000002 SizeRef=168,473 Selected=0xF5BE1C77\n"\
    "DockNode  ID=0x00000004 Parent=0x00000002 SizeRef=168,226 Selected=0xD3D12213\n"

namespace GE
{

Project::Project()
{
    ImGui::GetIO().IniFilename = nullptr;
    m_name = "new_project";
    m_ressourcesDir = "/";
    m_imguiSettings = DEFAULT_IMGUI_INI;
    m_imguiSettingsHasChanged = true;
}

Project::Project(const utils::String& filepath)
    : m_projFilePath(filepath)
{
    ImGui::GetIO().IniFilename = nullptr;
    reloadProject();
}

void Project::reloadProject()
{
    std::ifstream f(m_projFilePath);
    json jsn = json::parse(f);

    auto nameIt = jsn.find("name");
    if (nameIt != jsn.end())
        m_name = utils::String(nameIt->template get<std::string>().c_str());

    auto ressourcesDirIt = jsn.find("ressourcesDir");
    if (ressourcesDirIt != jsn.end())
        m_ressourcesDir = utils::String(ressourcesDirIt->template get<std::string>().c_str());

    auto imguiSettingsIt = jsn.find("imguiSettings");
    if (imguiSettingsIt != jsn.end())
        m_imguiSettings = utils::String(imguiSettingsIt->template get<std::string>().c_str());

    auto scenesIt = jsn.find("scenes");
    if (scenesIt != jsn.end())
    {
        for (auto& scene : *scenesIt)
            m_scenes.insert(scene.template get<Scene>());
    }

    auto startSceneNameIt = jsn.find("startScene");
    if (startSceneNameIt != jsn.end())  
    {
        utils::String startSceneName = utils::String(startSceneNameIt->template get<std::string>().c_str());
        auto startSceneIt = m_scenes.find(startSceneName);
        if (startSceneIt != m_scenes.end())
            m_startScene = &*startSceneIt;
    }

    m_imguiSettingsHasChanged = true;
}

void Project::saveProject()
{
    utils::String imguiSettings = ImGui::SaveIniSettingsToMemory();

    json jsn;

    jsn["name"] = std::string(m_name);
    jsn["ressourcesDir"] = std::string(m_ressourcesDir);
    jsn["imguiSettings"] = std::string(m_imguiSettings);

    json scenesJsn = json::array();
    for (auto& scene : m_scenes)
        scenesJsn.emplace_back(scene);
    jsn["scenes"] = scenesJsn;

    jsn["startScene"] = m_startScene ? m_startScene->name() : "";

    std::ofstream f(m_projFilePath);
    f << (jsn).dump(4);
}

void Project::loadimguiSettings()
{
    ImGui::LoadIniSettingsFromMemory((const char*)m_imguiSettings);
    m_imguiSettingsHasChanged = false;
}

}