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
#include "UtilsCPP/UniquePtr.hpp"

namespace GE
{

Project::Project()
    : m_projName("new_project"),
      m_game(utils::makeUnique<Game>())
{
}

Project::Project(const utils::String& filepath)
    : m_path(filepath)
{
}

void Project::reloadProject()
{
}

}