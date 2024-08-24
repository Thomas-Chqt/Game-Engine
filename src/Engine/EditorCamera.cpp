/*
 * ---------------------------------------------------
 * EditorCamera.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 13:57:19
 * ---------------------------------------------------
 */

#include "Engine/EditorCamera.hpp"
#include "Game-Engine/Input.hpp"
#include "Game-Engine/Mapper.hpp"
#include "Game-Engine/RawInput.hpp"
#include "InputManager/InputManagerIntern.hpp"
#include "Math/Constants.hpp"
#include "Math/Vector.hpp"
#include "Renderer/Renderer.hpp"
#include "UtilsCPP/UniquePtr.hpp"
#include <cmath>

namespace GE
{

void EditorCamera::setupFreeCam()
{
    Range2DInput& rotationInput = InputManagerIntern::shared().newEditorInput<Range2DInput>("editor_free_cam_rotate");
    rotationInput.setCallback([&](math::vec2f value) { m_rotation += math::vec3f{ value.x, value.y, 0.0F }.normalized() * 0.05; });

    Mapper<KeyboardButton, Range2DInput>::Descriptor camRotateMapperDescriptor;
    camRotateMapperDescriptor.xPos = KeyboardButton::down;
    camRotateMapperDescriptor.xNeg = KeyboardButton::up;
    camRotateMapperDescriptor.yPos = KeyboardButton::right;
    camRotateMapperDescriptor.yNeg = KeyboardButton::left;
    camRotateMapperDescriptor.input = &rotationInput;

    rotationInput.setMapper0(utils::UniquePtr<Mapper<KeyboardButton, Range2DInput>>(new Mapper<KeyboardButton, Range2DInput>(camRotateMapperDescriptor)).staticCast<IMapper>());


    Range2DInput& moveInput = InputManagerIntern::shared().newEditorInput<Range2DInput>("editor_free_cam_move");
    moveInput.setCallback([&](math::vec2f value) { m_position += math::mat3x3::rotation(m_rotation) * math::vec3f{ value.x, 0, value.y }.normalized() * 0.15; });

    Mapper<KeyboardButton, Range2DInput>::Descriptor camMoveMapperDescriptor;
    camMoveMapperDescriptor.xPos = KeyboardButton::d;
    camMoveMapperDescriptor.xNeg = KeyboardButton::a;
    camMoveMapperDescriptor.yPos = KeyboardButton::w;
    camMoveMapperDescriptor.yNeg = KeyboardButton::s;
    camMoveMapperDescriptor.input = &moveInput;

    moveInput.setMapper0(utils::UniquePtr<Mapper<KeyboardButton, Range2DInput>>(new Mapper<KeyboardButton, Range2DInput>(camMoveMapperDescriptor)).staticCast<IMapper>());
}

Renderer::Camera EditorCamera::getRendererCam()
{
    Renderer::Camera cam;

    float zs = 10000.0F / (10000.0F - 0.01F);
    float ys = 1.0F / std::tan((float)(60 * (PI / 180.0F)) * 0.5F);
    float xs = ys; // (ys / aspectRatio)

    cam.projectionMatrix = math::mat4x4(xs,  0,  0,           0,
                                         0, ys,  0,           0,
                                         0,  0, zs, -0.01F * zs,
                                         0,  0,  1,           0);

    cam.viewMatrix = (math::mat4x4::translation(m_position) * math::mat4x4::rotation(m_rotation)).inversed();
    return cam;
}

EditorCamera::~EditorCamera()
{
}

}