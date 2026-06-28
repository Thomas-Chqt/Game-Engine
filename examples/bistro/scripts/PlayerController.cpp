/*
 * ---------------------------------------------------
 * PlayerController.cpp
 * ---------------------------------------------------
 */

#include <Game-Engine/Components.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/Game.hpp>
#include <Game-Engine/Script.hpp>

#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>

#include <algorithm>
#include <cassert>
#include <cmath>

class PlayerController final : public GE::Script
{
    GE_SCRIPT(PlayerController, "PlayerController");

    GE_SCRIPT_PARAM(float, move_speed, 0.05f);
    GE_SCRIPT_PARAM(float, rotate_speed, 0.05f);
    GE_SCRIPT_PARAM(float, jump_height, 0.12f);
    GE_SCRIPT_PARAM(float, gravity, 0.006f);

    void setup(GE::Entity& entity, GE::Game& game) override
    {
        m_entity = entity;
        m_groundHeight = entity.get<GE::TransformComponent>().position.y;
        m_grounded = true;

        auto children = entity.children();
        auto it = std::ranges::find_if(children, [](const GE::Entity& child) -> bool { return child.name() == "camera"; });
        assert(it != children.end());

        const auto& cameraTransform = it->get<GE::TransformComponent>();
        const glm::vec3 cameraForward = cameraTransform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        m_cameraPitch = std::asin(std::clamp(cameraForward.y, -1.0f, 1.0f));

        game.inputContext().setInputCallback<GE::Range2DInput>(
            "player_move",
            [this, entity](const glm::vec2& value) mutable
            {
                auto& transform = entity.get<GE::TransformComponent>();
                const auto forward = transform.rotation * glm::vec3(0.0f, 0.0f, -1.0f);
                const auto right = transform.rotation * glm::vec3(1.0f, 0.0f, 0.0f);
                transform.position += (right * value.x + forward * value.y) * move_speed;
            }
        );

        game.inputContext().setInputCallback<GE::Range2DInput>(
            "player_rotate",
            [this, entity, camera=*it](const glm::vec2& value) mutable
            {
                auto& entityTransform = entity.get<GE::TransformComponent>();
                auto& cameraTransform = camera.get<GE::TransformComponent>();

                const glm::quat yawDelta = glm::angleAxis(value.y * rotate_speed, glm::vec3(0.0f, 1.0f, 0.0f));
                m_cameraPitch = std::clamp(m_cameraPitch + value.x * rotate_speed, glm::radians(-89.0f), glm::radians(89.0f));

                entityTransform.rotation = glm::normalize(yawDelta * entityTransform.rotation);
                cameraTransform.rotation = glm::angleAxis(m_cameraPitch, glm::vec3(1.0f, 0.0f, 0.0f));
            }
        );

        game.inputContext().setInputCallback<GE::ActionInput>(
            "player_jump",
            [this]()
            {
                if (!m_grounded)
                    return;

                m_verticalVelocity = jump_height;
                m_grounded = false;
            }
        );
    }

    void onUpdate() override
    {
        if (m_grounded)
            return;

        auto& transform = m_entity.get<GE::TransformComponent>();
        transform.position.y += m_verticalVelocity;
        m_verticalVelocity -= gravity;

        if (transform.position.y <= m_groundHeight)
        {
            transform.position.y = m_groundHeight;
            m_verticalVelocity = 0.0f;
            m_grounded = true;
        }
    }

private:
    GE::Entity m_entity;
    float m_groundHeight = 0.0f;
    float m_verticalVelocity = 0.0f;
    bool m_grounded = true;
    float m_cameraPitch = 0.0f;
};
