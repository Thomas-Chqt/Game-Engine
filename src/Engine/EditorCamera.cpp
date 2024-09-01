/*
 * ---------------------------------------------------
 * EditorCamera.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * Date: 2024/08/24 13:57:19
 * ---------------------------------------------------
 */

#include "Engine/EditorCamera.hpp"
#include "Math/Constants.hpp"
#include "Math/Vector.hpp"
#include "Renderer/Renderer.hpp"
#include <cmath>

namespace GE
{

void EditorCamera::rotate(math::vec2f value)
{
    m_rotation += math::vec3f{ value.x, value.y, 0.0F }.normalized() * 0.05;
}

void EditorCamera::move(math::vec2f value)
{
    m_position += math::mat3x3::rotation(m_rotation) * math::vec3f{ value.x, 0, value.y }.normalized() * 0.15;
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