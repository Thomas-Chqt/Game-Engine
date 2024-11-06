/*
 * ---------------------------------------------------
 * FileOpenDialog.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/04 17:24:23
 * ---------------------------------------------------
 */

#ifndef FILEOPENDIALOG_HPP
#define FILEOPENDIALOG_HPP

#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/String.hpp"
#include <filesystem>
#include <future>
#include <utility>

namespace GE
{

class FileOpenDialog
{
public:
    FileOpenDialog()                      = delete;
    FileOpenDialog(const FileOpenDialog&) = delete;
    FileOpenDialog(FileOpenDialog&&)      = delete;
    
    FileOpenDialog(const utils::String& title, bool& isPresented);

    inline FileOpenDialog& onSelection(const utils::Func<void(const std::filesystem::path&)>& f) { return m_onSelection = f, *this; }
    inline FileOpenDialog& onSelection(utils::Func<void(const std::filesystem::path&)>&& f) { return m_onSelection = std::move(f), *this; }

    inline FileOpenDialog& onCancel(const utils::Func<void()>& f) { return m_onCancel = f, *this; }
    inline FileOpenDialog& onCancel(utils::Func<void()>&& f) { return m_onCancel = std::move(f), *this; }

    void render();

    ~FileOpenDialog() = default;

private:
    utils::String m_title;
    bool& m_isPresented;
    inline static bool s_taskStarted = false;
    inline static std::future<char*> s_result;
    
    utils::Func<void(const std::filesystem::path&)> m_onSelection;
    utils::Func<void()> m_onCancel;

public:
    FileOpenDialog& operator = (const FileOpenDialog&) = delete;
    FileOpenDialog& operator = (FileOpenDialog&&)      = delete;
};

}

#endif // FILEOPENDIALOG_HPP