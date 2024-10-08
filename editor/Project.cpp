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
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

namespace GE
{

void to_json(json& jsn, const Project& project)
{
    jsn["name"] = std::string(project.name);
    jsn["ressourcesDir"] = std::string(project.ressourcesDir);
    jsn["imguiSettings"] = std::string(project.imguiSettings);

    json scenesJsn = json::array();
    for (auto& scene : project.scenes)
        scenesJsn.emplace_back(scene);
    jsn["scenes"] = scenesJsn;

    jsn["startScene"] = project.startScene ? project.startScene->name() : "";
}

void from_json(const json& jsn, Project& project)
{
    auto nameIt = jsn.find("name");
    project.name = nameIt != jsn.end() ? utils::String(nameIt->template get<std::string>().c_str()) : "";

    auto ressDirIt = jsn.find("ressourcesDir");
    project.ressourcesDir = ressDirIt != jsn.end() ? utils::String(ressDirIt->template get<std::string>().c_str()) : "";

    auto imguiSettIt = jsn.find("imguiSettings");
    if (imguiSettIt != jsn.end())
        project.imguiSettings = imguiSettIt != jsn.end() ? utils::String(imguiSettIt->template get<std::string>().c_str()) : utils::String(ImGui::SaveIniSettingsToMemory());

    auto scenesIt = jsn.find("scenes");
    if (scenesIt != jsn.end())
    {
        for (auto& scene : *scenesIt)
            project.scenes.insert(scene.template get<Scene>());
    }

    auto startSceneNameIt = jsn.find("startScene");
    if (startSceneNameIt != jsn.end())  
    {
        auto startSceneIt = project.scenes.find(utils::String(startSceneNameIt->template get<std::string>().c_str()));
        project.startScene = startSceneIt != project.scenes.end() ? &*startSceneIt : nullptr;
    }
    else
        project.startScene = nullptr;
}

}
