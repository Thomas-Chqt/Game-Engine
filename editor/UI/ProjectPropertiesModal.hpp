/*
 * ---------------------------------------------------
 * ProjectPropertiesModal.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/30 18:05:37
 * ---------------------------------------------------
 */

#ifndef PROJECTPROPERTIESMODAL_HPP
#define PROJECTPROPERTIESMODAL_HPP

#include "Project.hpp"

namespace GE
{

class ProjectPropertiesModal
{
public:
    ProjectPropertiesModal()                              = delete;
    ProjectPropertiesModal(const ProjectPropertiesModal&) = delete;
    ProjectPropertiesModal(ProjectPropertiesModal&&)      = delete;
    
    ProjectPropertiesModal(bool& isPresented, Project&);

    void render();

    ~ProjectPropertiesModal() = default;

private:
    bool isRessourceDirValid();
    bool isScriptLibValid();
    void closePopup();

    bool& m_isPresented;
    Project& m_project;

    inline static char s_nameBuff[32];
    inline static char s_ressourceDirBuff[1024];
    inline static char s_scriptsLibBuff[1024];
    inline static bool s_needBufUpdate = true;

public:
    ProjectPropertiesModal& operator = (const ProjectPropertiesModal&) = delete;
    ProjectPropertiesModal& operator = (ProjectPropertiesModal&&)      = delete;
};

}

#endif // PROJECTPROPERTIESMODAL_HPP