/*
 * ---------------------------------------------------
 * NewProjectPopupModal.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/08 09:22:15
 * ---------------------------------------------------
 */

#ifndef NEWPROJECTPOPUPMODAL_HPP
#define NEWPROJECTPOPUPMODAL_HPP

#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

class NewProjectPopupModal
{
public:
    using onCloseFunc = utils::Func<void(void)>;
    using onCreateFunc = utils::Func<void(void)>;

public:
    NewProjectPopupModal()                            = delete;
    NewProjectPopupModal(const NewProjectPopupModal&) = delete;
    NewProjectPopupModal(NewProjectPopupModal&&)      = delete;

    NewProjectPopupModal(bool& isPresented, utils::String& projectName, utils::String& path);
    
    inline NewProjectPopupModal& onClose(const onCloseFunc& f) { return m_onClose = f, *this; }
    inline NewProjectPopupModal& onCreatePressed(const onCreateFunc& f) { return m_onCreate = f, *this; }

    void render();

    ~NewProjectPopupModal() = default;

private:
    bool& m_isPresented;
    utils::String& m_projectName;
    utils::String& m_path;

    onCloseFunc m_onClose;
    onCreateFunc m_onCreate;

public:
    NewProjectPopupModal& operator = (const NewProjectPopupModal&) = delete;
    NewProjectPopupModal& operator = (NewProjectPopupModal&&)      = delete;
};

}

#endif // NEWPROJECTPOPUPMODAL_HPP