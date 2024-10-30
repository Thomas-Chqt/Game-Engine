/*
 * ---------------------------------------------------
 * Project.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/27 18:40:31
 * ---------------------------------------------------
 */

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "Scene.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"
#include <utility>
#include <filesystem>

namespace GE
{

class Project
{
public:
    Project();
    Project(const Project&) = default;
    Project(Project&&)      = default;

    Project(const std::filesystem::path&);
    
    inline bool hasSavePath() const { return m_savePath.empty() == false; }
    const std::filesystem::path& savePath() const { return m_savePath; }

    inline const utils::String& name() const { return m_name; }
    inline void setName(const utils::String& s) { m_name = s; }
    
    inline const std::filesystem::path& ressourcesDir() const { return m_ressourcesDir; }
    inline void setRessourcesDir(const std::filesystem::path& p) { m_ressourcesDir = p; }
    
    inline const std::filesystem::path& scriptLib() const { return m_scriptLib; }
    inline void setScriptLib(const std::filesystem::path& p) { m_scriptLib = p; }

    inline void loadIniSettingsFromMemory() const { ImGui::LoadIniSettingsFromMemory(m_imguiSettings); }
    inline void saveIniSettingsToMemory() { m_imguiSettings = ImGui::SaveIniSettingsToMemory(); }

    inline const utils::Set<Scene>& scenes() const { return m_scenes; }
    inline Scene& scene(const utils::String& name) { return *m_scenes.find(name); }

    inline void addScene(const Scene& s) { m_scenes.insert(s); }
    inline void addScene(Scene&& s) { m_scenes.insert(std::move(s)); }

    void deleteScene(const utils::String& name);
    inline void deleteScene(const Scene& s) { deleteScene(s.name()); }

    Scene* startScene();
    inline const Scene* startScene() const { return startScene(); }

    inline void setStartScene(const utils::String& name) { m_startScene = name; }
    inline void setStartScene(const Scene& s) { setStartScene(s.name()); }

    void reload();
    void save();
    void save(const std::filesystem::path&);

    ~Project() = default;

private:
    std::filesystem::path m_savePath;

    utils::String m_name;
    std::filesystem::path m_ressourcesDir;
    std::filesystem::path m_scriptLib;

    utils::String m_imguiSettings;

    utils::Set<Scene> m_scenes;
    utils::String m_startScene;

public:
    Project& operator = (const Project&) = default;
    Project& operator = (Project&&)      = default;

    friend void to_json(nlohmann::json&, const Project&);
    friend void from_json(const nlohmann::json&, Project&);
};

}

#endif // PROJECT_HPP