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
private:
    class Camera
    {
    public:
        Camera(GE::ECSWorld& world) : m_entity(world)
        {
            m_entity.add(GE::TransformComponent{ {0, 0, 0}, {0, 0, 0}, {1, 1, 1} });
            m_entity.add(GE::ViewPointComponent{ 60 * (PI / 180.0F), 0.1F, 10000 });
            m_entity.add(GE::ActiveViewPointComponent());
            m_entity.add(GE::ScriptComponent{ utils::Func<void(const utils::Set<int>&)>(*this, &Camera::onFrame) });
        }

        void onFrame(const utils::Set<int>& pressedKeys)
        {
        }

    private:
        GE::ECSWorld::Entity m_entity;
    };

    

public:
    Sandbox() : m_camera(m_defaultECSWorld)
    {
        m_windowWidth = 800;
        m_windowHeight = 600;
    }

    ~Sandbox() = default;

private:
    Camera m_camera;
};

utils::UniquePtr<GE::Game> createGame(int argv, char* argc[])
{
    return utils::makeUnique<Sandbox>().staticCast<GE::Game>();
}
