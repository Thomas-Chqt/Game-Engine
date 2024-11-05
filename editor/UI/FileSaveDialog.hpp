/*
 * ---------------------------------------------------
 * FileSaveDialog.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/05 11:18:07
 * ---------------------------------------------------
 */

#ifndef FILESAVEDIALOG_HPP
#define FILESAVEDIALOG_HPP

#include "UtilsCPP/String.hpp"
#include <filesystem>
#include <future>
#include <utility>

namespace GE
{

class FileSaveDialog
{
public:
    FileSaveDialog()                      = delete;
    FileSaveDialog(const FileSaveDialog&) = delete;
    FileSaveDialog(FileSaveDialog&&)      = delete;
    
    FileSaveDialog(const utils::String& defaultName, bool& isPresented);

    inline FileSaveDialog& onSelection(const utils::Func<void(const std::filesystem::path&)>& f) { return m_onSelection = f, *this; }
    inline FileSaveDialog& onSelection(utils::Func<void(const std::filesystem::path&)>&& f) { return m_onSelection = std::move(f), *this; }

    inline FileSaveDialog& onCancel(const utils::Func<void()>& f) { return m_onCancel = f, *this; }
    inline FileSaveDialog& onCancel(utils::Func<void()>&& f) { return m_onCancel = std::move(f), *this; }

    void render();

    ~FileSaveDialog() = default;

private:
    utils::String m_defaultName;
    bool& m_isPresented;
    inline static bool s_taskStarted = false;
    inline static std::future<char*> s_result;
    
    utils::Func<void(const std::filesystem::path&)> m_onSelection;
    utils::Func<void()> m_onCancel;

public:
    FileSaveDialog& operator = (const FileSaveDialog&) = delete;
    FileSaveDialog& operator = (FileSaveDialog&&)      = delete;
};

}

#endif // FILESAVEDIALOG_HPP