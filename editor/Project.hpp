/*
 * ---------------------------------------------------
 * Project.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/09/10 11:16:48
 * ---------------------------------------------------
 */

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "Scene.hpp"
#include "UtilsCPP/Set.hpp"
#include "UtilsCPP/String.hpp"
#include <nlohmann/json.hpp>

namespace GE
{

class Project
{
public:
    Project();
    Project(const Project&) = delete;
    Project(Project&&)      = default;
    
    Project(const utils::String&);

    // Getters / Setters
    inline const utils::String& projectFilePath() const { return m_projFilePath; }
    inline void setProjectFilePath(const utils::String& path) { m_projFilePath = path; }

    inline const utils::String& name() const { return m_name; }
    inline void setName(const utils::String& name) { m_name = name; }
    
    inline const utils::String& ressourcesDir() const { return m_ressourcesDir; }
    inline void setRessourceDir(const utils::String& s) { m_ressourcesDir = s; }

    //Utils
    inline utils::String baseDir() const { return m_projFilePath.isEmpty() ? "" : m_projFilePath.substr(0, m_projFilePath.lastIndexOf('/')); }
    inline utils::String ressourceDirFullPath() const { return baseDir() + "/" + m_ressourcesDir; }

    // Other
    void reloadProject();
    void saveProject();

    inline bool imguiSettingsHasChanged() const { return m_imguiSettingsHasChanged; }
    void loadimguiSettings();

    virtual ~Project() = default;

private:
    utils::String m_projFilePath;

    utils::String m_name;
    utils::String m_ressourcesDir;

    utils::String m_imguiSettings;
    bool m_imguiSettingsHasChanged = false;

    utils::Set<Scene> m_scenes;
    Scene* m_startScene = nullptr;
    
public:
    Project& operator = (const Project&) = delete;
    Project& operator = (Project&&)      = default;
};

}

#endif // PROJECT_HPP