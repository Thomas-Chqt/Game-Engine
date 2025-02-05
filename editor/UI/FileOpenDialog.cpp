/*
 * ---------------------------------------------------
 * FileOpenDialog.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/04 17:31:58
 * ---------------------------------------------------
 */

#include "UI/FileOpenDialog.hpp"
#include "TFD/tinyfiledialogs.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace GE
{

FileOpenDialog::FileOpenDialog(const utils::String& title, bool& isPresented)
    : m_isPresented(isPresented)
{
}

void FileOpenDialog::render()
{
    using namespace std::chrono_literals;

    static char title[32];

    if (m_isPresented == false)
        return;
    if (s_taskStarted == false)
    {
        m_title.safecpy(title, sizeof(title));
        s_result = std::async(tinyfd_openFileDialog, title, "", 0, nullptr, nullptr, 0);
        s_taskStarted = true;
    }
    if (s_result.wait_for(1ns) == std::future_status::ready)
    {
        char* path = s_result.get();
        if (path)
        {
            if (m_onSelection)
                m_onSelection(fs::path(path));
        }
        else if (m_onCancel)
            m_onCancel();

        m_isPresented = false;
        s_taskStarted = false;
    }    
}

}