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
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Game.hpp"
#include "Math/Constants.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/UniquePtr.hpp"

class Sandbox : public GE::Game
{
public:
    Sandbox()
    {
        m_camera.add(GE::TransformComponent{ {0, 0, 0}, {0, 0, 0}, {1, 1, 1} });
        m_camera.add(GE::ViewPointComponent{ 60 * (PI / 180.0F), 0.1F, 10000 });
        m_camera.add(GE::ActiveViewPointComponent());

        m_cube.add(GE::TransformComponent{ {0, 0, 5}, {0, 0, 0}, {1, 1, 1} });
        m_cube.add(GE::RenderableComponent{ GE::Engine::shared().loadMeshes(RESSOURCES_DIR"/cube.glb")[0] });

        m_light.add(GE::TransformComponent{ {0, 0, 0}, {0, 0, 0}, {1, 1, 1} });
        m_light.add(GE::LightSourceComponent{ GE::LightSourceComponent::Type::point, WHITE3, 1.0 });
    }

    ~Sandbox() = default;

private:
    GE::ECSWorld::Entity m_camera = m_defaultECSWorld;
    GE::ECSWorld::Entity m_cube = m_defaultECSWorld;
    GE::ECSWorld::Entity m_light = m_defaultECSWorld;
};

utils::UniquePtr<GE::Game> createGame(int argv, char* argc[])
{
    return utils::makeUnique<Sandbox>().staticCast<GE::Game>();
}
