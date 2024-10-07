/*
 * ---------------------------------------------------
 * ProjectPropertiesPanel.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/07 10:13:56
 * ---------------------------------------------------
 */

#ifndef PROJECTPROPERTIESPANEL_HPP
#define PROJECTPROPERTIESPANEL_HPP

#include "Project.hpp"
#include "UtilsCPP/Func.hpp"

namespace GE
{

class ProjectPropertiesPanel
{
public:
    ProjectPropertiesPanel()                              = delete;
    ProjectPropertiesPanel(const ProjectPropertiesPanel&) = delete;
    ProjectPropertiesPanel(ProjectPropertiesPanel&&)      = delete;
    
    ProjectPropertiesPanel(Project&);

    inline ProjectPropertiesPanel& onClose(const utils::Func<void()>& f) { return m_onClose = f, *this; }
    void render();

    ~ProjectPropertiesPanel() = default;

private:
    void projectNameEdit();

    Project& m_project;

    utils::Func<void()> m_onClose;

public:
    ProjectPropertiesPanel& operator = (const ProjectPropertiesPanel&) = delete;
    ProjectPropertiesPanel& operator = (ProjectPropertiesPanel&&)      = delete;
};

}

#endif // PROJECTPROPERTIESPANEL_HPP