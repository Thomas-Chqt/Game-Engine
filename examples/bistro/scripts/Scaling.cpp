/*
 * ---------------------------------------------------
 * Scaling.cpp
 * ---------------------------------------------------
 */

#include <Game-Engine/Components.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/Game.hpp>
#include <Game-Engine/Script.hpp>

class Scaling final : public GE::Script
{
    GE_SCRIPT(Scaling, "Scaling");
    GE_SCRIPT_PARAM(float, speed, 0.02f);
    GE_SCRIPT_PARAM(bool, increasing, true);

    void setup(GE::Entity& entity, GE::Game& game) override
    {
        (void)game;
        m_entity = entity;
    }

    void onUpdate() override
    {
        auto& transform = m_entity.get<GE::TransformComponent>();
        const float delta = increasing ? speed : -speed;
        transform.scale += glm::vec3(delta);

        if (transform.scale.x >= 5.0f)
            increasing = false;
        if (transform.scale.x <= 1.0f)
            increasing = true;
    }

private:
    GE::Entity m_entity;
};
