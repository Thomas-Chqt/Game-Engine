/*
 * ---------------------------------------------------
 * EditorCamera.cpp
 *
 * Author: Thomas Choquet <semoir.dense-0h@icloud.com>
 * ---------------------------------------------------
 */

#include "EditorCamera.hpp"

#include <Game-Engine/Scene.hpp>
#include <Game-Engine/Components.hpp>

#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <cassert>
#include <cmath>

namespace GE_Editor
{

namespace
{

constexpr float kMoveStepPerFrame = 5.0f / 60.0f;
constexpr float kRotateStepPerFrame = 1.5f / 60.0f;

bool isFinite(const glm::mat4& matrix)
{
    for (glm::length_t col = 0; col < matrix.length(); col++)
    {
        for (glm::length_t row = 0; row < matrix[col].length(); row++)
        {
            if (!std::isfinite(matrix[col][row]))
                return false;
        }
    }
    return true;
}

}

EditorCamera::EditorCamera(glm::vec3 pos, glm::quat rot)
    : m_position(pos)
{
    assert(glm::length(rot) != 0.0f);
    m_rotation = glm::normalize(rot);
}

glm::mat4 EditorCamera::viewMatrix() const
{
    auto rotationMat = rotationMatrix();

    glm::vec3 pos = m_position;
    glm::vec3 dir = rotationMat * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    glm::vec3 up = rotationMat * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

    return glm::lookAt(pos, pos + dir, up);
}

glm::mat4 EditorCamera::projectionMatrix(float aspectRatio) const
{
    return glm::perspective(m_fov, aspectRatio, m_zNear, m_zFar);
}

glm::mat4 EditorCamera::viewProjectionMatrix(float aspectRatio) const
{

    return projectionMatrix(aspectRatio) * viewMatrix();
}

void EditorCamera::setViewMatrix(const glm::mat4& viewMatrix)
{
    if (!isFinite(viewMatrix) || std::abs(glm::determinant(viewMatrix)) < 0.000001f)
        return;

    const glm::mat4 cameraTransform = glm::inverse(viewMatrix);
    if (!isFinite(cameraTransform))
        return;

    m_position = glm::vec3(cameraTransform[3]);
    m_rotation = glm::normalize(glm::quat_cast(glm::mat3(cameraTransform)));
}

void EditorCamera::onMoveInput(const glm::vec2& value)
{
    m_position += glm::vec3(rotationMatrix() * glm::vec4(glm::vec3{ value.x, 0.0f, -value.y } * kMoveStepPerFrame, 0.0f));
}

void EditorCamera::onRotationInput(const glm::vec2& value)
{
    const glm::quat yawDelta = glm::angleAxis(-value.y * kRotateStepPerFrame, glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::quat pitchDelta = glm::angleAxis(value.x * kRotateStepPerFrame, glm::vec3(1.0f, 0.0f, 0.0f));
    m_rotation = glm::normalize(yawDelta * m_rotation * pitchDelta);
}

glm::mat4 EditorCamera::rotationMatrix() const
{
    return glm::mat4_cast(m_rotation);
}

}
