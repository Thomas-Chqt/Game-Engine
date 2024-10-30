/*
 * ---------------------------------------------------
 * EditorUI.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/10/28 15:52:54
 * ---------------------------------------------------
 */

#ifndef EDITORUI_HPP
#define EDITORUI_HPP

#include "UtilsCPP/Types.hpp"
#include <filesystem>

namespace GE
{

class Editor;

class EditorUI
{
public:
    EditorUI()                = delete;
    EditorUI(const EditorUI&) = delete;
    EditorUI(EditorUI&&)      = delete;

    EditorUI(Editor&);

    utils::uint32 viewportPanelW() const;
    utils::uint32 viewportPanelH() const;

    void setFileExplorerPath(const std::filesystem::path&);

    void render();

    ~EditorUI() = default;

private:
    Editor& m_editor;

public:
    EditorUI& operator = (const EditorUI&) = delete;
    EditorUI& operator = (EditorUI&&)      = delete;
};

}

#endif // EDITORUI_HPP