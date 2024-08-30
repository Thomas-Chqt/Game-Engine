/*
 * ---------------------------------------------------
 * main.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/30 18:47:00
 * ---------------------------------------------------
 */

#include "Editor/Editor.hpp"
#include "Graphics/Platform.hpp"
#include <iostream>

int main(int argc, char* argv[])
{
    int returnCode = 0;

    gfx::Platform::init();
    {
        auto window = gfx::Platform::shared().newWindow(1280, 720);
        GE::Editor editor(window);
        if (argc > 2)
        {
            std::cerr << "Bad arguments" << std::endl;
            returnCode = 1;
        }
        else
        {
            if (argc == 2)
                editor.openProject(argv[1]);
            editor.run();
        }
    }
    gfx::Platform::terminate();

    return returnCode;
}