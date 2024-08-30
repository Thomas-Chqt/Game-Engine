/*
 * ---------------------------------------------------
 * Editor.hpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/30 18:45:16
 * ---------------------------------------------------
 */

#ifndef EDITOR_HPP
#define EDITOR_HPP

#include "Graphics/Window.hpp"
#include "UtilsCPP/SharedPtr.hpp"
#include "UtilsCPP/String.hpp"

namespace GE
{

class Editor
{
public:
    Editor(const utils::SharedPtr<gfx::Window>&);
    Editor(const Editor&) = default;
    Editor(Editor&&)      = default;

    void openProject(const utils::String& filePath);
    void run();

    ~Editor() = default;

private:
    
public:
    Editor& operator = (const Editor&) = default;
    Editor& operator = (Editor&&)      = default;
};

}

#endif // EDITOR_HPP