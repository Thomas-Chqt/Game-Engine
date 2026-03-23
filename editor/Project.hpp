/*
 * ---------------------------------------------------
 * Project.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 *
 * interface to the project file
 * load, reload, save, get values, set values
 *
 */

#ifndef PROJECT_HPP
#define PROJECT_HPP

#include "Game-Engine/Entity.hpp"
#include <Game-Engine/Scene.hpp>

#include <filesystem>
#include <list>
#include <type_traits>

namespace GE_Editor
{

class Project
{
public:
    Project(); // new project, not yet saved to disk
    Project(const Project&) = delete;
    Project(Project&&) = delete;

    Project(const std::filesystem::path&);

    inline auto& scenes(this auto&& self) { return self.m_scenes; }
    inline auto startScene(this auto&& self) -> std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, const GE::Scene*, GE::Scene*> { return self.m_startScene; }

    inline auto editedScene(this auto&& self) -> std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, const GE::Scene*, GE::Scene*> { return self.m_editedScene; }

    inline auto selectedEntity(this auto&& self) -> std::conditional_t<std::is_const_v<std::remove_reference_t<decltype(self)>>, GE::const_Entity, GE::Entity> { return self.m_selectedEntity; }
    inline void setSelectedEntity(const GE::Entity& e) { m_selectedEntity = e; }

private:
    std::filesystem::path m_projectFilePath;

    std::list<GE::Scene> m_scenes; // need no ref invalidation
    GE::Scene* m_startScene;

    GE::Scene* m_editedScene;
    GE::Entity m_selectedEntity;

public:
    Project& operator = (const Project&) = delete;
    Project& operator = (Project&&) = delete;
};

}

#endif // PROJECT_HPP
