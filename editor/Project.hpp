/*
 * ---------------------------------------------------
 * Project.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include <Game-Engine/Game.hpp>
#include <Game-Engine/Scene.hpp>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <utility>

namespace GE_Editor
{

class Project
{
public:
    Project(); // new project with default scene
    Project(const Project&) = delete;
    Project(Project&&) = delete;

    inline const std::string& name() const { return m_name; }
    inline void setName(const std::string& name) { m_name = name; }

    inline const std::map<uint32_t, GE::Scene::Descriptor>& scenes() const { return m_scenes; }
    inline void setScene(uint32_t id, const GE::Scene::Descriptor& desc) { m_scenes.insert_or_assign(id, desc); }

    inline std::pair<uint32_t, GE::Scene::Descriptor> startScene() const { return *m_scenes.find(m_startScene); }
    inline void setStartScene(uint32_t id) { assert(m_scenes.contains(id)); m_startScene = id; }

    inline const std::string& imguiSettings() const { return m_imguiSettings; }
    inline void setImguiSettings(std::string imguiSettings) { m_imguiSettings = std::move(imguiSettings); }

    inline const std::filesystem::path& scriptLib() const { return m_scriptLib; }
    inline void setScriptLib(std::filesystem::path scriptLib) { m_scriptLib = std::move(scriptLib); }

    GE::Game::Descriptor makeGameDescriptor() const;

private:
    std::string m_name;
    std::map<uint32_t, GE::Scene::Descriptor> m_scenes;
    uint32_t m_startScene;
    std::string m_imguiSettings;
    std::filesystem::path m_scriptLib;

public:
    Project& operator = (const Project&) = delete;
    Project& operator = (Project&&) = delete;

    friend struct YAML::convert<Project>;
};

}

namespace YAML
{

template<>
struct convert<GE_Editor::Project>
{
    static Node encode(const GE_Editor::Project& rhs)
    {
        Node node;
        node["name"] = rhs.m_name;
        node["imguiSettings"] = rhs.m_imguiSettings;
        node["scriptsLib"] = rhs.m_scriptLib.string();
        for (const auto& [_, scene] : rhs.m_scenes)
            node["scenes"].push_back(scene);
        node["startScene"] = rhs.startScene().second.name;
        return node;
    }

    static bool decode(const Node& node, GE_Editor::Project& rhs)
    {
        if (!node.IsMap() || !node["name"] || !node["startScene"])
            return false;
        if (!node["scenes"] || !node["scenes"].IsSequence() || node["scenes"].size() == 0)
            return false;

        rhs.m_name = node["name"].as<std::string>();
        rhs.m_imguiSettings = node["imguiSettings"] ? node["imguiSettings"].as<std::string>() : std::string();
        rhs.m_scriptLib = node["scriptsLib"] ? std::filesystem::path(node["scriptsLib"].as<std::string>()) : std::filesystem::path();

        rhs.m_scenes.clear();
        uint32_t sceneId = 0;
        for (const Node& sceneNode : node["scenes"])
            rhs.m_scenes.emplace(sceneId++, sceneNode.as<GE::Scene::Descriptor>());

        const std::string startSceneName = node["startScene"].as<std::string>();
        auto startScene = std::ranges::find_if(rhs.m_scenes, [&](auto& scene) -> bool {
            return scene.second.name == startSceneName;
        });
        if (startScene == rhs.m_scenes.end())
            return false;
        rhs.m_startScene = startScene->first;

        return true;
    }
};

}

#endif // PROJECT_HPP
