/*
 * ---------------------------------------------------
 * PlayerControl.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/08 16:22:51
 * ---------------------------------------------------
 */

#include "UtilsCPP/Func.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include "ECS/Entity.hpp"
#include "InputManager/Input.hpp"
#include "InputManager/Mapper.hpp"
#include "InputManager/RawInput.hpp"
#include "Script.hpp"
#include "ScriptLib.hpp"
#include "Game.hpp"

class PlayerControl : public GE::Script
{
public:
    using GE::Script::Script;

    PlayerControl(const GE::Entity& e, GE::Game& g) : GE::Script(e, g)
    {
        m_camera = m_entity.firstChild();

        GE::ActionInput& gameStopInput = m_game.inputContext().newInput<GE::ActionInput>("game_stop");
        gameStopInput.callback = utils::Func<void()>(m_game, &GE::Game::stop);
        auto gameStopInputMapper = utils::makeUnique<GE::Mapper<GE::KeyboardButton, GE::ActionInput>>(GE::KeyboardButton::esc, gameStopInput);
        gameStopInput.mappers[0] = gameStopInputMapper.staticCast<GE::IMapper>();

        GE::Mapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor inputMapperDesc;

        GE::Range2DInput& playerMoveIpt = m_game.inputContext().newInput<GE::Range2DInput>("player_move");
        playerMoveIpt.callback = utils::Func<void(math::vec2f)>(*this, &PlayerControl::move);
        inputMapperDesc.xPos = GE::KeyboardButton::d;
        inputMapperDesc.xNeg = GE::KeyboardButton::a;
        inputMapperDesc.yPos = GE::KeyboardButton::w;
        inputMapperDesc.yNeg = GE::KeyboardButton::s;
        auto playerMoveIptMapper = utils::makeUnique<GE::Mapper<GE::KeyboardButton, GE::Range2DInput>>(inputMapperDesc, playerMoveIpt);
        playerMoveIpt.mappers[0] = playerMoveIptMapper.staticCast<GE::IMapper>();
        playerMoveIpt.mappers[1].clear();

        GE::Range2DInput& playerRotateIpt = m_game.inputContext().newInput<GE::Range2DInput>("player_rotate");
        playerRotateIpt.callback = utils::Func<void(math::vec2f)>(*this, &PlayerControl::rotate);
        inputMapperDesc.xPos = GE::KeyboardButton::down;
        inputMapperDesc.xNeg = GE::KeyboardButton::up;
        inputMapperDesc.yPos = GE::KeyboardButton::right;
        inputMapperDesc.yNeg = GE::KeyboardButton::left;
        auto playerRotateIptMapper = utils::makeUnique<GE::Mapper<GE::KeyboardButton, GE::Range2DInput>>(inputMapperDesc, playerRotateIpt);
        playerRotateIpt.mappers[0] = playerRotateIptMapper.staticCast<GE::IMapper>();
        playerRotateIpt.mappers[1].clear();
    }

    void onUpdate() override
    {
    }

    void move(math::vec2f value)
    {
        m_entity.position() += math::mat3x3::rotation(m_entity.rotation()) * math::vec3f{ value.x, 0, value.y }.normalized() * 0.05;
    }

    void rotate(math::vec2f value)
    {
        m_entity.rotation() += math::vec3f{ 0.0F,    value.y, 0.0F }.normalized() * 0.025;
        m_camera.rotation() += math::vec3f{ value.x, 0.0F,    0.0F }.normalized() * 0.025;
    }

private:
    GE::Entity m_camera;
};

REGISTER_SCRIPT(PlayerControl);