/*
 * ---------------------------------------------------
 * PlayerControl.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/11/08 16:22:51
 * ---------------------------------------------------
 */

#define GFX_USING_GLFW

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

        GE::Mapper<GE::KeyboardButton, GE::Range2DInput>::Descriptor inputMapperDesc;

        GE::Range2DInput& editorCamMoveIpt = m_game.inputContext().newInput<GE::Range2DInput>("player_move");
        editorCamMoveIpt.callback = utils::Func<void(math::vec2f)>(*this, &PlayerControl::move);
        inputMapperDesc.xPos = GE::KeyboardButton::d;
        inputMapperDesc.xNeg = GE::KeyboardButton::a;
        inputMapperDesc.yPos = GE::KeyboardButton::w;
        inputMapperDesc.yNeg = GE::KeyboardButton::s;
        auto editorCamMoveIptMapper = utils::makeUnique<GE::Mapper<GE::KeyboardButton, GE::Range2DInput>>(inputMapperDesc, editorCamMoveIpt);
        editorCamMoveIpt.mappers[0] = editorCamMoveIptMapper.staticCast<GE::IMapper>();
        editorCamMoveIpt.mappers[1].clear();

        GE::Range2DInput& editorCamRotateIpt = m_game.inputContext().newInput<GE::Range2DInput>("player_rotate");
        editorCamRotateIpt.callback = utils::Func<void(math::vec2f)>(*this, &PlayerControl::rotate);
        inputMapperDesc.xPos = GE::KeyboardButton::down;
        inputMapperDesc.xNeg = GE::KeyboardButton::up;
        inputMapperDesc.yPos = GE::KeyboardButton::right;
        inputMapperDesc.yNeg = GE::KeyboardButton::left;
        auto editorCamRotateIptMapper = utils::makeUnique<GE::Mapper<GE::KeyboardButton, GE::Range2DInput>>(inputMapperDesc, editorCamRotateIpt);
        editorCamRotateIpt.mappers[0] = editorCamRotateIptMapper.staticCast<GE::IMapper>();
        editorCamRotateIpt.mappers[1].clear();
    }

    void onUpdate() override
    {
    }

    void move(math::vec2f value)
    {
        m_entity.position() += math::mat3x3::rotation(m_entity.rotation()) * math::vec3f{ value.x, 0, value.y }.normalized() * 0.15;
    }

    void rotate(math::vec2f value)
    {
        m_entity.rotation() += math::vec3f{ 0.0F,    value.y, 0.0F }.normalized() * 0.05;
        m_camera.rotation() += math::vec3f{ value.x, 0.0F,    0.0F }.normalized() * 0.05;
    }

private:
    GE::Entity m_camera;
};

REGISTER_SCRIPT(PlayerControl);