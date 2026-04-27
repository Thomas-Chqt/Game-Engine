/*
 * ---------------------------------------------------
 * PlayerController.cpp
 * ---------------------------------------------------
 */

#include <Game-Engine/Components.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/Game.hpp>
#include <Game-Engine/Script.hpp>

class PlayerController final : public GE::Script
{
    GE_SCRIPT(PlayerController, "PlayerController");

    void setup(GE::Entity& entity, GE::Game& game) override
    {
        m_entity = entity;
        game.inputContext().setInputCallback<GE::Range2DInput>(
            "player_input",
            [this](const glm::vec2& value)
            {
                auto& transform = m_entity.get<GE::TransformComponent>();
                transform.position.x += value.x * 0.05f;
                transform.position.z -= value.y * 0.05f;
            }
        );
    }

    void teardown(GE::Entity&, GE::Game& game) override
    {
        game.inputContext().setInputCallback<GE::Range2DInput>("player_input", {});
    }

private:
    GE::Entity m_entity;
};
