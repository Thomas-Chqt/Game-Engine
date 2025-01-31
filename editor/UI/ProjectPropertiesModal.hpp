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
#include "UtilsCPP/Func.hpp"

namespace GE
{

class ProjectPropertiesModal
{
public:
    ProjectPropertiesModal()                              = delete;
    ProjectPropertiesModal(const ProjectPropertiesModal&) = delete;
    ProjectPropertiesModal(ProjectPropertiesModal&&)      = delete;
    
    ProjectPropertiesModal(bool& isPresented, Project&, const std::filesystem::path& projSavePath);

    inline ProjectPropertiesModal& onOk(const utils::Func<void()>& f) { return m_onOk = f, *this; }
    inline ProjectPropertiesModal& onOk(utils::Func<void()>&& f) { return m_onOk = std::move(f), *this; }
    
    inline ProjectPropertiesModal& onCancel(const utils::Func<void()>& f) { return m_onCancel = f, *this; }
    inline ProjectPropertiesModal& onCancel(utils::Func<void()>&& f) { return m_onCancel = std::move(f), *this; }

    void render();

    ~ProjectPropertiesModal() = default;

private:
    bool isScriptLibValid();
    void closePopup();

    bool& m_isPresented;
    Project& m_project;
    const std::filesystem::path& m_projSavePath;

    utils::Func<void()> m_onOk;
    utils::Func<void()> m_onCancel;

    inline static char s_nameBuff[32];
    inline static char s_scriptsLibBuff[1024];
    inline static bool s_needBufUpdate = true;

public:
    ProjectPropertiesModal& operator = (const ProjectPropertiesModal&) = delete;
    ProjectPropertiesModal& operator = (ProjectPropertiesModal&&)      = delete;
};

}

#endif // PROJECTPROPERTIESMODAL_HPP