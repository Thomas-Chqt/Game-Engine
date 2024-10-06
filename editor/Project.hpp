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
#include "UtilsCPP/UniquePtr.hpp"
#include "Game.hpp"
#include <nlohmann/json.hpp>

namespace GE
{

class Project
{
public:
    Project();
    Project(const Project&) = delete;
    Project(Project&&)      = default;
    
    Project(const utils::String& filepath);

    inline const utils::String& path() const { return m_path; }
    inline void setPath(const utils::String& path) { m_path = path; }
    
    inline const utils::String& name() const { return m_projName; }
    inline void setName(const utils::String& name) { m_projName = name; }

    void reloadProject();
    void saveProject();

    inline Game& game() { return *m_game; }

    inline void setRessourceDir(const utils::String& s) { m_ressourcesDir = s; }
    inline utils::String ressourceDirFullPath() const { return m_path + "/" + m_ressourcesDir; }

    virtual ~Project() = default;

private:
    utils::String m_path;

    utils::String m_projName;
    utils::String m_ressourcesDir;

    utils::Set<Scene> m_scenes;
    utils::UniquePtr<Game> m_game;
    
public:
    Project& operator = (const Project&) = delete;
    Project& operator = (Project&&)      = default;

    friend void to_json(nlohmann::json&, const Project&);
    friend void from_json(const nlohmann::json&, Project&);
};

}

#endif // PROJECT_HPP