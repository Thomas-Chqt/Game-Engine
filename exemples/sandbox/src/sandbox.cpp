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
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include "Graphics/KeyCodes.hpp"
#include "Game-Engine/Entity.hpp"

class Player
{
public:
    Player(GE::ECSWorld& world) : m_entity(world)
    {
        m_entity.add(GE::DebugNameComponent{ "Player" });
        m_entity.add(GE::TransformComponent{ {0, 0, 0}, {0, 0, 0}, {1, 1, 1} });
        m_entity.add(GE::ViewPointComponent{ 60 * (PI / 180.0F), 0.1F, 10000 });
        m_entity.add(GE::ActiveViewPointComponent());
        m_entity.add(GE::ScriptComponent{ utils::Func<void(const utils::Set<int>&)>(*this, &Player::onLoop) });
    }

    void onLoop(const utils::Set<int>& pressedKeys)
    {
        GE::TransformComponent& transformComponent = m_entity.get<GE::TransformComponent>();    
        math::vec3f dir = { 0.0, 0.0, 0.0 };
        for (const auto& key : pressedKeys)
        {
            switch (key)
            {
                case W_KEY:     dir += math::vec3f{ 0, 0,  1};         break;
                case A_KEY:     dir += math::vec3f{-1, 0,  0};         break;
                case S_KEY:     dir += math::vec3f{ 0, 0, -1};         break;
                case D_KEY:     dir += math::vec3f{ 1, 0,  0};         break;
                case UP_KEY:    transformComponent.rotation.x -= 0.05; break;
                case LEFT_KEY:  transformComponent.rotation.y -= 0.05; break;
                case DOWN_KEY:  transformComponent.rotation.x += 0.05; break;
                case RIGHT_KEY: transformComponent.rotation.y += 0.05; break;
            }
        }
        transformComponent.position += math::mat3x3::rotation(transformComponent.rotation) * dir.normalized() * 0.2;
    }

private:
    GE::Entity m_entity;
};

class Sandbox : public GE::Game
{
public:
    Sandbox()
    {
        GE::Engine::shared().showEditorUI();

        m_cube.add(GE::DebugNameComponent{ "Cube" });
        m_cube.add(GE::TransformComponent{ {0, 0, 5}, {0, 0, 0}, {1, 1, 1} });
        m_cube.add(GE::RenderableComponent{ GE::Engine::shared().loadMeshes(RESSOURCES_DIR"/cube.glb")[0] });

        m_light.add(GE::DebugNameComponent{ "Light" });
        m_light.add(GE::TransformComponent{ {0, 0, 0}, {0, 0, 0}, {1, 1, 1} });
        m_light.add(GE::LightSourceComponent{ GE::LightSourceComponent::Type::point, WHITE3, 1.0 });
    }

    void onKeyDownEvent(int keyCode, bool isRepeat) override
    {
        if (keyCode == ESC_KEY)
            GE::Engine::shared().terminateGame();
    }

    ~Sandbox() = default;

private:
    Player m_player = m_defaultECSWorld;
    GE::Entity m_cube = m_defaultECSWorld;
    GE::Entity m_light = m_defaultECSWorld;
};

utils::UniquePtr<GE::Game> createGame(int argv, char* argc[])
{
    return utils::makeUnique<Sandbox>().staticCast<GE::Game>();
}
