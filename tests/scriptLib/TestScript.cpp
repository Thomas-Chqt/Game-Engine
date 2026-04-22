#include <Game-Engine/Components.hpp>
#include <Game-Engine/Entity.hpp>
#include <Game-Engine/Game.hpp>
#include <Game-Engine/Script.hpp>

class TestScript final : public GE::Script
{
    GE_SCRIPT(TestScript, "TestScript");
    GE_SCRIPT_PARAM(float, speed, 2.5f);
    GE_SCRIPT_PARAM(bool, enabled, true);
    GE_SCRIPT_PARAM(std::string, label, std::string("default"));

    void setup(GE::Entity& entity, GE::Game& game) override
    {
        (void)game;
        wasSetup = entity.has<GE::TransformComponent>();
    }

    void onUpdate() override
    {
        updateCount++;
    }

    bool wasSetup = false;
    int64_t updateCount = 0;
};
