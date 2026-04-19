/*
 * ---------------------------------------------------
 * ProjectPropertiesPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#ifndef PROJECTPROPERTIESPANEL_HPP
#define PROJECTPROPERTIESPANEL_HPP

#include "Project.hpp"

#include <filesystem>

namespace GE_Editor
{

class ProjectPropertiesPanel
{
public:
    ProjectPropertiesPanel() = delete;
    ProjectPropertiesPanel(const ProjectPropertiesPanel&) = delete;
    ProjectPropertiesPanel(ProjectPropertiesPanel&&) = delete;

    ProjectPropertiesPanel(Project*, std::filesystem::path*, bool*);

    void render();

    ~ProjectPropertiesPanel() = default;

private:
    Project* m_project = nullptr;
    std::filesystem::path* m_projectFilePath = nullptr;
    bool* m_isOpen = nullptr;

public:
    ProjectPropertiesPanel& operator=(const ProjectPropertiesPanel&) = delete;
    ProjectPropertiesPanel& operator=(ProjectPropertiesPanel&&) = delete;
};

} // namespace GE_Editor

#endif // PROJECTPROPERTIESPANEL_HPP
