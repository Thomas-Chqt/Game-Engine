/*
 * ---------------------------------------------------
 * Project.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/11 12:07:15
 * ---------------------------------------------------
 */

#include "Project.hpp"
#include "Game.hpp"
#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace GE
{

Project::Project()
    : m_projName("new_project"),
      m_ressourcesDir("/"),
      m_scenes(),
      m_game(utils::makeUnique<Game>(m_scenes))
{
}

Project::Project(const utils::String& filepath)
{
    m_path = filepath;
    reloadProject();
}

void Project::reloadProject()
{
    std::ifstream f(m_path);
    Project reloadedProject = json::parse(f);
    reloadedProject.m_path = m_path;
    *this = std::move(reloadedProject);
}

void Project::saveProject()
{
    std::ofstream f(m_path);
    f << ((json)*this).dump(4);
}

void to_json(nlohmann::json& json, const Project& project)
{
    json["name"] = std::string(project.m_projName);
    json["ressourcesDir"] = std::string(project.m_ressourcesDir);
}

void from_json(const nlohmann::json& json, Project& project)
{
    project.m_projName = utils::String(json["name"].template get<std::string>().c_str());
    project.m_ressourcesDir = utils::String(json["ressourcesDir"].template get<std::string>().c_str());
}

}