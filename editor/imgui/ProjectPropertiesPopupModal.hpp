/*
 * ---------------------------------------------------
 * ProjectPropertiesPopupModal.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/07 10:13:56
 * ---------------------------------------------------
 */

#ifndef PROJECTPROPERTIESPOPUPMODAL_HPP
#define PROJECTPROPERTIESPOPUPMODAL_HPP

#include "Project.hpp"
#include "UtilsCPP/Func.hpp"

namespace GE
{

class ProjectPropertiesPopupModal
{
public:
    using onCloseFunc = utils::Func<void(void)>;

public:
    ProjectPropertiesPopupModal()                              = delete;
    ProjectPropertiesPopupModal(const ProjectPropertiesPopupModal&) = delete;
    ProjectPropertiesPopupModal(ProjectPropertiesPopupModal&&)      = delete;
    
    ProjectPropertiesPopupModal(bool& isPresented, Project&);

    inline ProjectPropertiesPopupModal& onClose(const onCloseFunc& f) { return m_onClose = f, *this; }
    void render();

    ~ProjectPropertiesPopupModal() = default;

private:
    void projectNameEdit();
    void resourceDirEdit();

    bool& m_isPresented;
    Project& m_project;

    onCloseFunc m_onClose;

public:
    ProjectPropertiesPopupModal& operator = (const ProjectPropertiesPopupModal&) = delete;
    ProjectPropertiesPopupModal& operator = (ProjectPropertiesPopupModal&&)      = delete;
};

}

#endif // PROJECTPROPERTIESPOPUPMODAL_HPP