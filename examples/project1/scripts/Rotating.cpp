/*
 * ---------------------------------------------------
 * Rotating.cpp
 * ---------------------------------------------------
 */

#include <Game-Engine/Components.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/Game.hpp>
#include <Game-Engine/Script.hpp>

#include <glm/gtc/quaternion.hpp>

class Rotating final : public GE::Script
{
    GE_SCRIPT(Rotating, "Rotating");
    GE_SCRIPT_PARAM(float, speed, 0.02f);

    void setup(GE::Entity& entity, GE::Game& game) override
    {
        (void)game;
        m_entity = entity;
    }

    void onUpdate() override
    {
        auto& transform = m_entity.get<GE::TransformComponent>();
        transform.rotation = glm::normalize(transform.rotation * glm::angleAxis(speed, glm::vec3(0.0f, 1.0f, 0.0f)));
    }

private:
    GE::Entity m_entity;
};
