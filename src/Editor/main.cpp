/*
 * ---------------------------------------------------
 * main.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/30 18:47:00
 * ---------------------------------------------------
 */

#include "Editor/Editor.hpp"
#include "Engine/Engine.hpp"
#include "Graphics/Platform.hpp"

int main(int argc, char* argv[])
{
    int returnCode = 0;

    gfx::Platform::init();
    GE::Engine::init(gfx::Platform::shared().newWindow(1280, 720));

    GE::Engine::terminate();
    gfx::Platform::terminate();

    return returnCode;
}