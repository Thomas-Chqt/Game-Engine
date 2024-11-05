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

FileOpenDialog::FileOpenDialog(bool& isPresented)
    : m_isPresented(isPresented)
{
}

void FileOpenDialog::render()
{
    using namespace std::chrono_literals;

    if (m_isPresented == false)
        return;
    if (s_taskStarted == false)
    {
        s_result = std::async(tinyfd_openFileDialog, "Open project", "", 0, nullptr, nullptr, 0);
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