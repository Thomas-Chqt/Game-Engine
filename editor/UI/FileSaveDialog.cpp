/*
 * ---------------------------------------------------
 * FileSaveDialog.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/05 11:22:57
 * ---------------------------------------------------
 */

#include "UI/FileSaveDialog.hpp"
#include "TFD/tinyfiledialogs.h"
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

namespace GE
{

FileSaveDialog::FileSaveDialog(const utils::String& defaultName, bool& isPresented)
    : m_isPresented(isPresented), m_defaultName(defaultName)
{
}

void FileSaveDialog::render()
{
    using namespace std::chrono_literals;

    static char defaultFileName[32];

    if (m_isPresented == false)
        return;
    if (s_taskStarted == false)
    {
        m_defaultName.safecpy(defaultFileName, sizeof(defaultFileName));
        s_result = std::async(tinyfd_saveFileDialog, "Choose destination", defaultFileName, 0, nullptr, nullptr);
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