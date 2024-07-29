/*
 * ---------------------------------------------------
 * sandbox.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:38:24
 * ---------------------------------------------------
 */

#include "Game-Engine/Components.hpp"
#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Game.hpp"
#include "Math/Constants.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/UniquePtr.hpp"

class Sandbox : public GE::Game
{
public:
    Sandbox() : m_camera(m_defaultECSWorld)
    {
        m_windowWidth = 800;
        m_windowHeight = 600;
    }

    ~Sandbox() = default;

private:
    struct Camera
    {
        GE::ECSWorld& world;
        GE::EntityID id;

        Camera(GE::ECSWorld& world) : world(world)
        {
            world.createEntity();
            world.addComponent(id, GE::TransformComponent{ {0, 0, 0}, {0, 0, 0}, {1, 1, 1} });
            world.addComponent(id, GE::ViewPointComponent{ 60 * (PI / 180.0F), 0.1F, 10000 });
            world.addComponent(id, GE::ActiveViewPointComponent());
            world.addComponent(id, GE::ScriptComponent{ utils::Func<void(const utils::Set<int>&)>(*this, &Sandbox::Camera::onFrame) });
        }

        void onFrame(const utils::Set<int>& pressedKeys)
        {
        }
    };

    Camera m_camera;
};

utils::UniquePtr<GE::Game> createGame(int argv, char* argc[])
{
    return utils::makeUnique<Sandbox>().staticCast<GE::Game>();
}
