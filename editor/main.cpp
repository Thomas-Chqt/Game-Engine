/*
 * ---------------------------------------------------
 * main.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/30 18:47:00
 * ---------------------------------------------------
 */

#include <Game-Engine/Application.hpp>

#include <print>

class Editor : public GE::Application
{
public:
    void onUpdate() override
    {
        std::println("on update");
    }

    void onEvent(GE::Event& event) override
    {
        if (event.dispatch<GE::WindowRequestCloseEvent>([&](GE::WindowRequestCloseEvent&) {
            terminate();
        })) return;
    }
};

int main(int argc, char* argv[])
{
    Editor().run();
}
