/*
 * ---------------------------------------------------
 * main.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/30 18:47:00
 * ---------------------------------------------------
 */

#include "Engine/Engine.hpp"
#include "Graphics/Platform.hpp"

int main(int argc, char* argv[])
{
    gfx::Platform::init();
    GE::Engine::init();

    GE::Engine::shared().run();

    GE::Engine::terminate();
    gfx::Platform::terminate();
}