/*
 * ---------------------------------------------------
 * Project.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/27 18:41:09
 * ---------------------------------------------------
 */

#include "Project.hpp"
#include "ECS/Components.hpp"
#include "Scene.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>

#define DEFAULT_IMGUI_INI                                                                                                 \
    "[Window][WindowOverViewport_11111111]\nPos=0,19\nSize=1280,701\nCollapsed=0\n\n"                                     \
    "[Window][Debug##Default]\nPos=60,60\nSize=400,400\nCollapsed=0\n\n"                                                  \
    "[Window][Entity inspector]\nPos=1015,405\nSize=265,315\nCollapsed=0\nDockId=0x00000004,0\n\n"                        \
    "[Window][Scene graph]\nPos=1015,19\nSize=265,384\nCollapsed=0\nDockId=0x00000003,0\n\n"                              \
    "[Window][viewport]\nPos=0,19\nSize=1013,554\nCollapsed=0\nDockId=0x00000007,0\n\n"                                   \
    "[Window][File explorer]\nPos=0,544\nSize=1013,176\nCollapsed=0\nDockId=0x00000006,0\n\n"                             \
    "[Window][Project properties]\nPos=435,288\nSize=361,77\nCollapsed=0\n\n"                                             \
    "[Window][Scenes]\nPos=445,309\nSize=268,102\nCollapsed=0\n\n"                                                        \
    "[Window][Content browser]\nPos=0,575\nSize=1013,145\nCollapsed=0\nDockId=0x00000008,0\n\n"                           \
    "[Docking][Data]\n"                                                                                                   \
    "DockSpace       ID=0x7C6B3D9B Window=0xA87D555D Pos=100,196 Size=1280,701 Split=X Selected=0x0BA3B4F3\n"             \
    "  DockNode      ID=0x00000001 Parent=0x7C6B3D9B SizeRef=1428,701 Split=Y Selected=0x0BA3B4F3\n"                      \
    "      DockNode    ID=0x00000005 Parent=0x00000001 SizeRef=1013,523 Split=Y Selected=0x0BA3B4F3\n"                    \
    "            DockNode  ID=0x00000007 Parent=0x00000005 SizeRef=1013,554 CentralNode=1 Selected=0x0BA3B4F3\n"          \
    "                  DockNode  ID=0x00000008 Parent=0x00000005 SizeRef=1013,145 Selected=0x3995E0EF\n"                  \
    "                      DockNode    ID=0x00000006 Parent=0x00000001 SizeRef=1013,176 Selected=0xD2F73F3F\n"            \
    "                        DockNode      ID=0x00000002 Parent=0x7C6B3D9B SizeRef=265,701 Split=Y Selected=0xF5BE1C77\n" \
    "                            DockNode    ID=0x00000003 Parent=0x00000002 SizeRef=168,583 Selected=0xF5BE1C77\n"       \
    "                                DockNode    ID=0x00000004 Parent=0x00000002 SizeRef=168,479 Selected=0xD3D12213\n\n" \

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace GE
{

Project::Project()
    : m_name("new_project"),
      m_imguiSettings(DEFAULT_IMGUI_INI)
{
    auto& defautScene = *m_scenes.insert(Scene("default_scene"));
    auto cube = defautScene.newEntity("cube");
    cube.emplace<TransformComponent>();
    cube.emplace<MeshComponent>(BUILT_IN_CUBE_ASSET_ID);

    auto light = defautScene.newEntity("light");
    light.emplace<TransformComponent>(math::vec3f{-1.5, 1.5, -1.5}, math::vec3f{0, 0, 0}, math::vec3f{0, 0, 0});
    light.emplace<LightComponent>();

    m_startScene = "default_scene";
}

Project::Project(const fs::path& filePath)
{
    assert(fs::is_regular_file(filePath));
    *this = json::parse(std::ifstream(filePath));
}

void Project::deleteScene(const utils::String& name)
{
    assert(m_startScene != name);
    m_scenes.remove(m_scenes.find(name));
}

Scene* Project::startScene()
{
    auto it = m_scenes.find(m_startScene);
    if (it != m_scenes.end())
        return &*it;
    else
        return nullptr;
}

void Project::save(const fs::path& filePath)
{
    assert(filePath.is_absolute());
    assert(fs::is_directory(fs::path(filePath).remove_filename()));
    std::ofstream(filePath) << json(*this).dump(4);
}

void to_json(json& jsn, const Project& project)
{
    jsn["name"] = std::string(project.m_name);
    jsn["ressourcesDir"] = project.m_ressourcesDir.string();
    jsn["scriptsLib"] = project.m_scriptLib.string();

    jsn["imguiSettings"] = std::string(project.m_imguiSettings);

    json scenesJsn = json::array();
    for (const auto& scene : project.m_scenes)
        scenesJsn.emplace_back(scene);
    jsn["scenes"] = scenesJsn;

    jsn["startScene"] = project.m_startScene;
}

void from_json(const json& jsn, Project& project)
{
    auto nameIt = jsn.find("name");
    project.m_name = nameIt != jsn.end() ? utils::String(nameIt->template get<std::string>().c_str()) : "";

    auto ressDirIt = jsn.find("ressourcesDir");
    project.m_ressourcesDir = ressDirIt != jsn.end() ? ressDirIt->template get<fs::path>() : fs::path();

    auto scriptsLibIt = jsn.find("scriptsLib");
    project.m_scriptLib = scriptsLibIt != jsn.end() ? scriptsLibIt->template get<fs::path>() : fs::path();

    auto imguiSettIt = jsn.find("imguiSettings");
    if (imguiSettIt != jsn.end())
        project.m_imguiSettings = utils::String(imguiSettIt->template get<std::string>().c_str());
    else
        project.m_imguiSettings = ImGui::SaveIniSettingsToMemory();

    project.m_scenes.clear();
    auto scenesIt = jsn.find("scenes");
    if (scenesIt != jsn.end())
    {
        for (auto& scene : *scenesIt)
            project.m_scenes.insert(scene.template get<Scene>());
    }

    auto startSceneNameIt = jsn.find("startScene");
    if (startSceneNameIt != jsn.end() && project.m_scenes.contain(utils::String(startSceneNameIt->template get<std::string>().c_str())))  
        project.m_startScene = utils::String(startSceneNameIt->template get<std::string>().c_str());
    else
        project.m_startScene = "";
}

}