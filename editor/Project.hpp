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

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <map>
#include <utility>

namespace GE_Editor
{

class Project
{
public:
    Project(); // new project with defautl scene
    Project(const Project&) = delete;
    Project(Project&&) = delete;

    Project(const std::filesystem::path&);

    inline const std::map<uint32_t, GE::Scene::Descriptor>& scenes() const { return m_scenes; }
    inline void setScene(uint32_t id, const GE::Scene::Descriptor& desc) { m_scenes.insert_or_assign(id, desc); }

    inline std::pair<uint32_t, GE::Scene::Descriptor> startScene() const { return *m_scenes.find(m_startScene); }
    inline void setStartScene(uint32_t id) { assert(m_scenes.contains(id)); m_startScene = id; }

    GE::Game::Descriptor gameDescriptor() const;

private:
    std::map<uint32_t, GE::Scene::Descriptor> m_scenes;
    uint32_t m_startScene;

public:
    Project& operator = (const Project&) = delete;
    Project& operator = (Project&&) = delete;
};

}

#endif // PROJECT_HPP
