/*
 * ---------------------------------------------------
 * main.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/30 18:47:00
 * ---------------------------------------------------
 */

#include "Editor.hpp"
#include "Graphics/Platform.hpp"
#include "UtilsCPP/UniquePtr.hpp"

int main(int argc, char* argv[])
{
    gfx::Platform::init();

    auto editor = utils::makeUnique<GE::Editor>();
    editor->run();
    editor.clear();

    gfx::Platform::terminate();
}