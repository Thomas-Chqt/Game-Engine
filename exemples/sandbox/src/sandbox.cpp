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
#include "Game-Engine/ECSWorld.hpp"
#include "Game-Engine/Engine.hpp"
#include "Game-Engine/Entity.hpp"
#include "Game-Engine/Game.hpp"
#include "Game-Engine/Input.hpp"
#include "Game-Engine/Mapper.hpp"
#include "Game-Engine/RawInput.hpp"
#include "Math/Vector.hpp"
#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include "Math/Constants.hpp"
#include <iostream>

class Player : public GE::Entity
{
public:
    using GE::Entity::Entity;
    Player(const GE::Entity& e) : GE::Entity(e) {}

    static GE::Entity create(GE::ECSWorld& scene)
    {
        GE::Entity entity = scene.newEntity("player");
        entity.emplace<GE::CameraComponent>((float)(60 * (PI / 180.0F)), 10000.0f, 0.01f);
        entity.emplace<GE::ActiveCameraComponent>();
        entity.emplace<GE::LightComponent>(GE::LightComponent::Type::point, WHITE3, 1.0f);
        entity.setScriptInstance<Player>();
        return entity;
    }

    void onSetup() override
    {
    }

    void onUpdate() override
    {
    }

    void rotate(math::vec2f value)
    {
        rotation() += math::vec3f{ value.x, value.y, 0.0F }.normalized() * 0.05;
    }

    void move(math::vec2f value)
    {
        position() += math::mat3x3::rotation(rotation()) * math::vec3f{ value.x, 0, value.y }.normalized() * 0.15;
    }
};

class Cube : public GE::Entity
{
public:
    using GE::Entity::Entity;

    static GE::Entity create(GE::ECSWorld& scene)
    {
        GE::Entity entity = scene.newEntity("cube");
        entity.emplace<GE::MeshComponent>(GE::AssetManager::shared().getMesh(RESSOURCES_DIR"/cube.glb"));
        entity.get<GE::TransformComponent>().position = {0, 0, 5};
        entity.setScriptInstance<Cube>();
        return entity;
    }

    void onUpdate() override
    {
        GE::TransformComponent& transformComponent = get<GE::TransformComponent>();

        transformComponent.rotation.x += + 0.5 * (PI / 180.0F);
        if (transformComponent.rotation.x > 2*PI)
            transformComponent.rotation.x -= 2*PI;

        transformComponent.rotation.y += + 0.7 * (PI / 180.0F);
        if (transformComponent.rotation.y > 2*PI)
            transformComponent.rotation.y -= 2*PI;
    };
};

class Sandbox : public GE::Game
{
public:
    Sandbox()
    {
        GE::Entity player = Player::create(m_defaultScene);
        GE::Entity cube = Cube::create(m_defaultScene);

        GE::Entity chess_set = m_defaultScene.newEntity("chess_set");
        chess_set.emplace<GE::MeshComponent>(GE::AssetManager::shared().getMesh(RESSOURCES_DIR"/chess_set/chess_set.gltf"));
        chess_set.scale() = {5, 5, 5};

        cube.pushChild(chess_set);

        GE::ActionInput& terminateGameIpt = m_inputContext.newInput<GE::ActionInput>("terminate_game");
        terminateGameIpt.callback = utils::Func<void()>(GE::Engine::shared(), &GE::Engine::terminateGame);
        auto terminateGameIptMapper = utils::makeUnique<GE::Mapper<GE::KeyboardButton, GE::ActionInput>>(GE::KeyboardButton::esc, terminateGameIpt);
        terminateGameIpt.mappers[0] = terminateGameIptMapper.staticCast<GE::IMapper>();

        GE::Range2DInput& rotationInput = m_inputContext.newInput<GE::Range2DInput>("rotate_player");
        rotationInput.callback = [player](math::vec2f value) { Player(player).rotate(value); };
        GE::Mapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor mapperDesc;
        mapperDesc.xPos = GE::KeyboardButton::down;
        mapperDesc.xNeg = GE::KeyboardButton::up;
        mapperDesc.yPos = GE::KeyboardButton::right;
        mapperDesc.yNeg = GE::KeyboardButton::left;
        auto rotationInputMapper = utils::makeUnique<GE::Mapper<GE::KeyboardButton, GE::Range2DInput>>(mapperDesc, rotationInput);
        rotationInput.mappers[0] = rotationInputMapper.staticCast<GE::IMapper>();

        GE::Range2DInput& moveInput = m_inputContext.newInput<GE::Range2DInput>("move_player");
        moveInput.callback = [player](math::vec2f value) { Player(player).move(value); };
        mapperDesc.xPos = GE::KeyboardButton::d;
        mapperDesc.xNeg = GE::KeyboardButton::a;
        mapperDesc.yPos = GE::KeyboardButton::w;
        mapperDesc.yNeg = GE::KeyboardButton::s;
        auto moveInputMapper = utils::makeUnique<GE::Mapper<GE::KeyboardButton, GE::Range2DInput>>(mapperDesc, moveInput);
        moveInput.mappers[0] = moveInputMapper.staticCast<GE::IMapper>();
    }

    void onSetup() override
    {
        std::cout << "Game setup" << std::endl;
    }

    void onUpdate() override
    {
    }
};

int main()
{
    GE::Engine::init();

    // GE::Engine::shared().runGame(utils::makeUnique<Sandbox>().staticCast<GE::Game>());
    GE::Engine::shared().editorForGame(utils::makeUnique<Sandbox>().staticCast<GE::Game>());

    GE::Engine::terminate();
}
