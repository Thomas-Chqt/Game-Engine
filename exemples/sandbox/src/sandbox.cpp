/*
 * ---------------------------------------------------
 * sandbox.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/06/20 16:38:24
 * ---------------------------------------------------
 */

#include "Game-Engine/AssetManager.hpp"
#include "Game-Engine/Components.hpp"
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Game.hpp"
#include "Graphics/Event.hpp"
#include "Graphics/KeyCodes.hpp"
#include "Graphics/Platform.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include "Math/Constants.hpp"
#include <iostream>

class Player : public GE::Entity
{
public:
    Player(GE::Entity entity) : GE::Entity(entity)
    {
        emplace<GE::CameraComponent>((float)(60 * (PI / 180.0F)), 10000.0f, 0.01f);
        emplace<GE::ActiveCameraComponent>();
        emplace<GE::LightComponent>(GE::LightComponent::Type::point, WHITE3, 1.0f);
    }

    void onUpdate() override
    {
        GE::TransformComponent& transformComponent = get<GE::TransformComponent>();    
        math::vec3f dir = { 0.0, 0.0, 0.0 };
        for (const auto& key : GE::Engine::shared().pressedKeys())
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
    };
};

class Cube : public GE::Entity
{
public:
    Cube(GE::Entity entity) : GE::Entity(entity)
    {
        emplace<GE::MeshComponent>(GE::AssetManager::shared().getMesh(RESSOURCES_DIR"/cube.glb"));
        get<GE::TransformComponent>().position = {0, 0, 5};
    }

    void onUpdate() override
    {
        GE::TransformComponent& transformComponent = get<GE::TransformComponent>();

        transformComponent.rotation.x += + 0.5 * (PI / 180.0F);
        if (transformComponent.rotation.x > 2*PI)
            transformComponent.rotation.x -= 2*PI;

        transformComponent.rotation.y += + 0.7 * (PI / 180.0F);
        if (transformComponent.rotation.x > 2*PI)
            transformComponent.rotation.x -= 2*PI;
    };
};

class Sandbox : public GE::Game
{
public:
    Sandbox()
    {
        m_defaultScene.makeEntityScriptable(utils::makeUnique<Player>(m_defaultScene.newEntity("player")).staticCast<GE::Entity>());
        m_defaultScene.makeEntityScriptable(utils::makeUnique<Cube>(m_defaultScene.newEntity("cube")).staticCast<GE::Entity>());

        GE::Entity chess_set = m_defaultScene.newEntity("chess_set");
        chess_set.emplace<GE::MeshComponent>(GE::AssetManager::shared().getMesh(RESSOURCES_DIR"/chess_set/chess_set.gltf"));
        chess_set.scale() = {5, 5, 5};
    }

    void onSetup() override
    {
        std::cout << "Game setup" << std::endl;
    }

    void onEvent(gfx::Event& event) override
    {
        if (event.dispatch<gfx::KeyDownEvent>([](gfx::KeyDownEvent& event) { 
            if (event.keyCode() == ESC_KEY)
                GE::Engine::shared().terminateGame();
        })) return;

        GE::Game::onEvent(event);
    }
};

int main()
{
    GE::Engine::init();

    GE::Engine::shared().runGame(utils::makeUnique<Sandbox>().staticCast<GE::Game>());

    GE::Engine::terminate();
}
