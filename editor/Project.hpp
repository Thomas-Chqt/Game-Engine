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

#include "UtilsCPP/String.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include "Game.hpp"

namespace GE
{

class Project
{
public:
    Project()               = delete;
    Project(const Project&) = delete;
    Project(Project&&)      = default;
    
    Project(const utils::String& filepath);

    void reloadProject();

    ~Project() = default;

private:
    utils::String m_path;

    utils::String m_name;

    utils::UniquePtr<Game> m_game;

    Scene* m_editedScene = nullptr;
    
    
public:
    Project& operator = (const Project&) = delete;
    Project& operator = (Project&&)      = default;
};

}

#endif // PROJECT_HPP