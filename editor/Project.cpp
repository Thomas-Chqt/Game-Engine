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

namespace GE
{

Project::Project()
{
    ImGui::GetIO().IniFilename = nullptr;
    m_iniSettingsNeedLoad = true;
}

Project::Project(const utils::String& filepath)
    : m_path(filepath)
{
    ImGui::GetIO().IniFilename = nullptr;
    reloadProject();
}

void Project::reloadProject()
{
    std::ifstream f(m_path);
    json jsn = json::parse(f);

    auto nameIt = jsn.find("name");
    if (nameIt != jsn.end())
        m_projName = utils::String(nameIt->template get<std::string>().c_str());

    auto ressourcesDirIt = jsn.find("ressourcesDir");
    if (ressourcesDirIt != jsn.end())
        m_ressourcesDir = utils::String(ressourcesDirIt->template get<std::string>().c_str());

    auto imguiIniIt = jsn.find("imguiIni");
    if (imguiIniIt != jsn.end())
        m_imguiIni = utils::String(imguiIniIt->template get<std::string>().c_str());

    auto scenesIt = jsn.find("scenes");
    if (scenesIt != jsn.end())
    {
        for (auto& scene : *scenesIt)
            m_scenes.insert(scene.template get<Scene>());
    }

    auto startSceneIt = jsn.find("startScene");
    if (startSceneIt != jsn.end())  
        utils::String startSceneName = utils::String(startSceneIt->template get<std::string>().c_str());

    m_iniSettingsNeedLoad = true;
}

void Project::saveProject()
{
    m_imguiIni = ImGui::SaveIniSettingsToMemory();

    json jsn;

    jsn["name"] = std::string(m_projName);
    jsn["ressourcesDir"] = std::string(m_ressourcesDir);
    jsn["imguiIni"] = std::string(m_imguiIni);

    json scenesJsn = json::array();
    for (auto& scene : m_scenes)
        scenesJsn.emplace_back(scene);

    jsn["scenes"] = scenesJsn;



    std::ofstream f(m_path);
    f << (jsn).dump(4);
}

void Project::loadIniSettings()
{
    ImGui::LoadIniSettingsFromMemory((const char*)m_imguiIni);
    m_iniSettingsNeedLoad = false;
}

}